#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <semaphore.h>

#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

#include <netinet/in.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include "helpers.h"

void showUsage();
void quitter(int signo);
void cleanup(void);
char* validateName(char* name);
int validateType(char* type);
char* validatePath(char* path);
int validateAddress(char* ip);
short validatePort(short port);
int connectLocal(char* path);
int connectWeb(int address, short port);
void handleReq();
void sendMessage(char type, int ctn, int result);
void registerClient(char* name, int sfd);

int sfd; // socket file descriptor
char* name;
char connectType;

int main(int argc, char** argv){
  if((argc != 4 ) && (argc != 5)) showUsage();
  name = validateName(argv[1]);
  int type = validateType(argv[2]);

  if(signal(SIGINT, quitter) == SIG_ERR) throw("Signal failed!");
  if(atexit(cleanup) != 0) throw("At exit failed!"); // procedura sprzatajaca

  if((type == LOCAL) || (argc == 4)){
    char* path = validatePath(argv[3]);
    sfd = connectLocal(path);
  }else if((type == WEB) || (argc == 5)){
    int address = validateAddress(argv[3]);
    short port = validatePort((short)atoi(argv[4]));
    sfd = connectWeb(address, port);
  }else showUsage();

  registerClient(name, sfd);

  while(1){
    char mType;
    if(read(sfd, &mType, 1) != 1) throw("CLIENT: reading type failed!");

    if(mType == PING){
      sendMessage(PONG, 0, 0);
    }else if(mType == REQ){
      handleReq();
    }
  }
}

void handleReq(){
  char command;
  int ctn, op1, op2, result;

  if(read(sfd, &command, 1) != 1) throw("CLIENT: reading command failed!");
  if(read(sfd, &ctn, sizeof(int)) != sizeof(int)) throw("CLIENT: reading ctn failed!");
  if(read(sfd, &op1, sizeof(int)) != sizeof(int)) throw("CLIENT: reading op1 failed!");
  if(read(sfd, &op2, sizeof(int)) != sizeof(int)) throw("CLIENT: reading op2 failed!");

  op1 = ntohl(op1);
  op2 = ntohl(op2);

  switch(command){
    case '+':
      result = op1 + op2;
    break;
    case '-':
      result = op1 - op2;
    break;
    case '/':
      result = op1 / op2;
    break;
    case '*':
      result = op1 * op2;
    break;
    default:
      printf("CLIENT: unknown command!");
      result = 0;
    break;
  }

  result = htonl(result);
  sendMessage(RESULT, ctn, result);
}

void sendMessage(char type, int ctn, int result){
  Message mess;
  mess.type = type;
  for(int i=0; i<21; i++) mess.name[i] = 0;
  for(int i=0; i<strlen(name); i++){
    mess.name[i] = name[i];
  }
  mess.ctn = ctn;
  mess.result = result;
  mess.connectType = connectType;
  if(write(sfd, &mess, sizeof(Message)) != sizeof(Message)) throw("CLIENT: sending message failed!");
}

void registerClient(char* name, int sfd){
  sendMessage(LOGIN, 0, 0);

  char mType;
  printf("Waiting for login response...\n");
  if(read(sfd, &mType, 1) != 1) throw("Receiving login response failed!");

  if(mType == FAILSIZE){
    printf("Too many clients, couldnt log in!\n");
    exit(2);
  }else if(mType == FAILNAME){
    printf("Name already in use, couldnt log in!\n");
    exit(2);
  }else if(mType == SUCCESS){
    printf("Logged in successfully!\n");
  }else{
    throw("Unpredicted behaviour in registerClient!");
  }

}

int connectLocal(char* path){
  struct sockaddr_un localAddress;
  localAddress.sun_family = AF_UNIX;
  for(int i=0; i<strlen(path) + 1; i++){
    localAddress.sun_path[i] = path[i];
  }

  int localSocket = socket(AF_UNIX, SOCK_DGRAM, 0);
  if(localSocket == -1) throw("CLIENT: creating localSocket failed!");

  if(bind(localSocket, &localAddress, sizeof(sa_family_t)) == -1) throw("local abstract binding failed!");
  if(connect(localSocket, &localAddress, sizeof(localAddress)) == -1) throw("CLIENT: localSocket connecting failed!");

  connectType = LOCAL;

  return localSocket;
}
int connectWeb(int address, short port){
  int webSocket = socket(AF_INET, SOCK_DGRAM, 0);
  if(webSocket == -1) throw("Creating webSocket failed!");

  struct sockaddr_in webAddress;
  webAddress.sin_family = AF_INET;
  webAddress.sin_port = 0;
  webAddress.sin_addr.s_addr = INADDR_ANY;

  if(bind(webSocket, &webAddress, sizeof(webAddress)) == -1) throw("CLIENT: webSocket binding failed!");
  webAddress.sin_family = AF_INET;
  webAddress.sin_port = htons(port);
  webAddress.sin_addr.s_addr = htonl(address);
  if(connect(webSocket, &webAddress, sizeof(webAddress)) == -1) throw("CLIENT: webSocket connecting failed!");

  connectType = WEB;

  return webSocket;
}

void showUsage(){
  printf("Use program like <name> <type> <address or path> (<port>)\n");
  exit(1);
}

char* validateName(char* name){
  int l = strlen(name);
  if((l < 2) || (l > 20)){
    throw("Name must be of length between 2 and 20");
  }
  return name;
}
int validateType(char* type){
  if(strcmp(type, "loc") == 0){
    return LOCAL;
  }else if(strcmp(type, "web") == 0){
    return WEB;
  }
  else{
    throw("Type must be loc or web!");
    return -1;
  }
}
char* validatePath(char* path){
  int l = strlen(path);
  if((l < 1) || (l > UNIX_PATH_MAX)){
    printf("Path must be of length between 1 and %d\n", UNIX_PATH_MAX);
    exit(1);
  }
  return path;
}
int validateAddress(char* ip){
  int x = inet_addr(ip);
  if(x == -1) throw("CLIENT: inet_addr failed! Adrees is wrong");
  return x;
}
short validatePort(short port){
  if((port < 1024) || (port > 60999)){
    throw("Port must be a number between 1024 and 60999!");
  }
  return port;
}

void quitter(int signo){
  sendMessage(LOGOUT, 0, 0);
  exit(2);
}

void cleanup(void){
  int allOk = 1;

  if(close(sfd) == -1){
    printf("Error closing client socket! Errno: %d, %s\n", errno, strerror(errno));
    allOk = 0;
  }

  if(allOk == 1) printf("All closed down!\n");
}
