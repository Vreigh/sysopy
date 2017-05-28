#define _GNU_SOURCE
#define UNIX_PATH_MAX    108

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

#include "helpers.h"

void showUsage();
int validatePort(short port);
char* validatePath(char* path);
void quitter(int signo);
void cleanup(void);
int getWebSocket(short portNum);
int getLocalSocket(char* path);
void* pingerJob(void*);
int prepareMonitor();
void handleNewRequest(int socket);
void handleNewMessage(int fd);
void tryRegister(int fd, char* name);

int webSocket;
int localSocket;
char* unixPath;

int counter = 0;
int cN = 0;
Client clients[50];
int epoll;
pthread_t pinger;
pthread_t commander;
pthread_mutex_t clientsLock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char** argv){
  if(argc != 3) showUsage();
  if(signal(SIGINT, quitter) == SIG_ERR) throw("Signal failed!");
  if(atexit(cleanup) != 0) throw("At exit failed!"); // procedura sprzatajaca

  short portNum = validatePort((short)atoi(argv[1])); // walidacja poprawnosci inputu
  unixPath = validatePath(argv[2]);

  webSocket = getWebSocket(portNum); // adres, socket, bindowanie i listen
  localSocket = getLocalSocket(unixPath);

  epoll = prepareMonitor();

  if(pthread_create(&pinger, NULL, pingerJob, NULL) != 0) throw("Creating thread failed!");

  struct epoll_event event;
  while(1){
    if(epoll_wait(epoll, &event, 1, -1) == -1) throw("Epoll_wait failed!");

    if(event.data.fd < 0){
      handleNewRequest(-event.data.fd);
    }else{ // nowa wiadomosc
      handleNewMessage(event.data.fd);
    }

  }
}
void handleNewRequest(int socket){
  int newClient = accept(socket, NULL, NULL);
  if(newClient == -1) throw("accepting new client failed!");

  struct epoll_event event;
  event.events = EPOLLIN | EPOLLPRI;
  event.data.fd = newClient;

  if(epoll_ctl(epoll, EPOLL_CTL_ADD, newClient, &event) == -1) throw("ctl add for new client failed!");
}

void handleNewMessage(int fd){
  char mType;
  short mLen;

  if(read(fd, &mType, 1) != 1) throw("reading new message failed!");
  if(read(fd, &mLen, 2) != 2) throw("reading new message failed!");
  mLen = ntohs(mLen);

  char* name = malloc(mLen);

  if(mType == LOGIN){
    if(read(fd, name, mLen) != mLen) throw("reading new message failed!");
    tryRegister(fd, name);
  }else if(mType == RESULT){
    int resultCtn, result;
    if(read(fd, &resultCtn, sizeof(int)) != sizeof(int)) throw("reading new message failed!");
    resultCtn = ntohl(resultCtn);

    if(read(fd, &result, sizeof(int)) != sizeof(int)) throw("reading new message failed!");
    result = ntohl(result);

    if(read(fd, name, mLen) != mLen) throw("reading new message failed!");

    printf("Client %s calculated task %d with teh result of %d!\n", name, resultCtn, result);
  }

  free(name);
}

void tryRegister(int fd, char* name){
  if(cN == 50){
    // przygotuj i wyslij wiadomosc do kliena
    // wyrzuc go z epolla
  }
  int ok = 1;

  pthread_mutex_lock(&clientsLock);
  for(int i=0; i<cN; i++){
    if(strcmp(clients[i].name, name) == 0){
      ok = 0;
      break;
    }
  }
  if(ok == 0){
    // przygotuj i wyslij wiadomosc do klienta, mowiaca, ze nie mozna go zarejestrowac
    // nastepnie wyrzuc go z epolla
  }else{
    clients[cN].fd = fd;
    clients[cN].name = concat("", name);
    cN++; // juz jest zarejestrowany w epollu - to wszystko
  }
  pthread_mutex_unlock(&clientsLock);
}

void* pingerJob(void* arg){
  while(1){
    printf("Hello, its just pinger, you know, pinging...\n");
    sleep(1);
  }
  return NULL;
}

int prepareMonitor(){
  int epoll = epoll_create(0);
  if(epoll == -1) throw("Creating epoll monitor failed!");

  struct epoll_event event;
  event.events = EPOLLIN | EPOLLPRI;
  event.data.fd = -webSocket;
  if(epoll_ctl(epoll, EPOLL_CTL_ADD, webSocket, &event) == -1) throw("ctl for webSocket failed!");

  event.data.fd = -localSocket;
  if(epoll_ctl(epoll, EPOLL_CTL_ADD, localSocket, &event) == -1) throw("ctl for localSocket failed!");

  return epoll;
}

int getWebSocket(short portNum){
  struct sockaddr_in webAddress;
  webAddress.sin_family = AF_INET;
  webAddress.sin_port = htons(portNum);
  webAddress.sin_addr.s_addr = INADDR_ANY;

  int webSocket = socket(AF_INET, SOCK_STREAM, 0);
  if(webSocket == -1) throw("Creating webSocket failed!");

  if(bind(webSocket, &webAddress, sizeof(webAddress)) == -1) throw("webSocket binding failed!");
  if(listen(webSocket, 50) == -1) throw("webSocket listen failed!");

  return webSocket;
}

int getLocalSocket(char* path){
  struct sockaddr_un localAddress;
  localAddress.sun_family = AF_UNIX;
  for(int i=0; i<strlen(path) + 1; i++){
    localAddress.sun_path[i] = path[i];
  }

  int localSocket = socket(AF_UNIX, SOCK_STREAM, 0);
  if(localSocket == -1) throw("Creating localSocket failed!");

  if(bind(localSocket, &localAddress, sizeof(localAddress)) == -1) throw("localSocket binding failed!");
  if(listen(localSocket, 50) == -1) throw("localSocket listen failed!");

  return localSocket;
}

int validatePort(short port){
  if((port < 1024) || (port > 60999)){
    throw("Port must be a number between 1024 and 60999!");
  }
  return port;
}

char* validatePath(char* path){
  int l = strlen(path);
  if((l < 1) || (l > UNIX_PATH_MAX)){
    printf("Port must be of length between 1 and %d\n", UNIX_PATH_MAX);
    exit(1);
  }
  return path;
}

void quitter(int signo){
  exit(2);
}

void cleanup(void){
  int allOk = 1;

  if(close(webSocket) == -1){
    printf("Error closing webSocket! Errno: %d, %s\n", errno, strerror(errno));
    allOk = 0;
  }
  if(close(localSocket) == -1){
    printf("Error closing localSocket! Errno: %d, %s\n", errno, strerror(errno));
    allOk = 0;
  }
  if(unlink(unixPath) == -1){
    printf("Unlinking unixPath failed! Errno: %d, %s\n", errno, strerror(errno));
    allOk = 0;
  }
  if(close(epoll) == -1){
    printf("Closing epoll failed! Errno: %d, %s\n", errno, strerror(errno));
    allOk = 0;
  }

  if(allOk == 1) printf("All cleaned up!\n");
}

void showUsage(){
  printf("Use program like <portNum> <unixPath>\n");
  exit(1);
}
