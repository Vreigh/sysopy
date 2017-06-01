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
short validatePort(short port);
char* validatePath(char* path);
void quitter(int signo);
void cleanup(void);
int getWebSocket(short portNum);
int getLocalSocket(char* path);
void* pingerJob(void*);
void* commanderJob(void*);
int prepareMonitor();
void handleNewMessage(int fd);
void tryRegister(int fd,struct sockaddr* addr, socklen_t aSize, Message* mess);
void removeClient(int i);
int pushCommand(char command, int op1, int op2, char* name);
Client* findClient(char* name);
int getSFD(char type);

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
  if(pthread_create(&commander, NULL, commanderJob, NULL) != 0) throw("Creating thread failed!");

  struct epoll_event event;
  while(1){
    if(epoll_wait(epoll, &event, 1, -1) == -1) throw("Epoll_wait failed!");
    handleNewMessage(event.data.fd);
  }
}

void handleNewMessage(int fd){
  struct sockaddr* addr = malloc(ASIZE);
  socklen_t aSize = sizeof(struct sockaddr);
  Message mess;

  if(recvfrom(fd, &mess, sizeof(Message), 0, addr, &aSize) != sizeof(Message))
    throw("receiving new message failed!");

  if(mess.type == LOGIN){
    tryRegister(fd, addr, aSize, &mess);
  }else if(mess.type == RESULT){
    int resultCtn = ntohl(mess.ctn);
    int result = ntohl(mess.result);

    printf("Client %s calculated task %d with the result of %d!\n", mess.name, resultCtn, result);
  }else if(mess.type == PONG){
    pthread_mutex_lock(&clientsLock); // zablokuj, zeby nie scigac sie z pingerem
    for(int i=0; i<cN; i++){ // przeszukaj, czy pinger juz nie usunal tej nazwy
      if(strcmp(clients[i].name, mess.name) == 0){
        clients[i].rec++; // jesli pinger jeszcze nie usunal, to zwieksz licznik otrzymanych
      }
    }
    pthread_mutex_unlock(&clientsLock);
  }else if(mess.type == LOGOUT){
    pthread_mutex_lock(&clientsLock); // zablokuj, zeby nie scigac sie z pingerem
    for(int i=0; i<cN; i++){
      if(strcmp(clients[i].name, mess.name) == 0){ // faktycznie istnieje taki klient, ktory chcialby sie wylogowac
        printf("Client %s logged out!\n", mess.name);
        removeClient(i);
      }
    }
    pthread_mutex_unlock(&clientsLock);
  }
}

void tryRegister(int fd, struct sockaddr *addr, socklen_t aSize, Message* mess){
  char mType;
  pthread_mutex_lock(&clientsLock);
  if(cN == 50){
    mType = FAILSIZE;
    if(sendto(fd, &mType, 1, 0, addr, aSize)!= 1) throw("Failsize message failed!");
    free(addr);
  }else{
    Client* client = findClient(mess->name);
    if(client != NULL){
      mType = FAILNAME;
      if(sendto(fd, &mType, 1, 0, addr, aSize)!= 1) throw("Failname message failed!");
      free(addr);
    }else{
      clients[cN].addr = addr;
      clients[cN].name = concat("", mess->name);
      clients[cN].sent = 0;
      clients[cN].rec = 0;
      clients[cN].type = mess->connectType;
      clients[cN].aSize = aSize;
      cN++;
      mType = SUCCESS;
      if(sendto(fd, &mType, 1, 0, addr, aSize) != 1) throw("Success message failed!");
    }
  }
  pthread_mutex_unlock(&clientsLock);
}

void* pingerJob(void* arg){
  while(1){
    //printf("ping!\n");
    pthread_mutex_lock(&clientsLock);
    int j = 0;
    for(int i=0; i<cN; i++){
      if(clients[j].sent != clients[j].rec){
        printf("Woha! Removing %s!\n", clients[j].name);
        removeClient(j); // usuwam klienta i shiftuje tablice, wiec potem chce jeszcze raz ten sam indeks sprawzic
      }else{
        char mType = PING;
        int sfd = getSFD(clients[j].type);
        if(sendto(sfd, &mType, 1, 0, clients[j].addr, clients[j].aSize) != 1){
          printf("There has been an error sending PING to client %s\n", clients[j].name);
        }
        clients[j].sent++;
        j++;
      }
    }
    pthread_mutex_unlock(&clientsLock);
    sleep(1);
  }
  return NULL;
}

void removeClient(int i){
  free(clients[i].addr);
  free(clients[i].name);
  for(int j = i; j + 1 < cN; j++){
    clients[j] = clients[j + 1];
  }
  cN--;
}

void* commanderJob(void* arg){
  char command;
  int op1, op2;
  char name[20];
  char buff[100];
  while(1){
    printf("Enter commands in format <op1> <command> <op2> <name>!\n");
    fgets(buff, 100, stdin);
    if(sscanf(buff, "%d %c %d %s\n", &op1, &command, &op2, name) != 4){
      printf("Wrong format of input!\n");
      continue;
    }
    if((command != '+') && (command != '-') && (command != '/') && (command != '*')){
      printf("Wrong command!\n");
      continue;
    }

    int res = pushCommand(command, op1, op2, name);
    if(res == 0){
      printf("Command %d: %d %c %d Has been sent to client %s!\n",counter -1, op1, command, op2, name);
    }else if(res == -1){
      printf("Command couldnt be sent. Client not found!\n");
    }else{
      printf("Command couldnt be sent. Something weird happened...\n");
    }
  }
  return NULL;
}

int pushCommand(char command, int op1, int op2, char* name){
  pthread_mutex_lock(&clientsLock);
  Client* client = findClient(name);

  if(client == NULL){
    pthread_mutex_unlock(&clientsLock);
    return -1;
  }

  int sfd = getSFD(client->type);
  char mType = REQ;
  int currCtn = htonl(counter++);
  int ope1 = htonl(op1);
  int ope2 = htonl(op2);
  int ok = 0;

  if(sendto(sfd, &mType, 1, 0, client->addr, client->aSize) != 1) ok = -2;
  if(sendto(sfd, &command, 1, 0, client->addr, client->aSize) != 1) ok = -2;
  if(sendto(sfd, &currCtn, sizeof(int), 0, client->addr, client->aSize) != sizeof(int)) ok = -2;
  if(sendto(sfd, &ope1, sizeof(int),0, client->addr, client->aSize) != sizeof(int)) ok = -2;
  if(sendto(sfd, &ope2, sizeof(int),0,client->addr, client->aSize) != sizeof(int)) ok = -2;

  pthread_mutex_unlock(&clientsLock);

  return ok;
}

Client* findClient(char* name){
  for(int i=0; i<cN; i++){
    if(strcmp(clients[i].name, name) == 0){
      return &clients[i];
    }
  }
  return NULL;
}

int getSFD(char type){
  if(type == LOCAL){
    return localSocket;
  }else if(type == WEB){
    return webSocket;
  }
  return -1;
}

int prepareMonitor(){
  int epoll = epoll_create1(0);
  if(epoll == -1) throw("Creating epoll monitor failed!");

  struct epoll_event event;
  event.events = EPOLLIN | EPOLLPRI;
  event.data.fd = webSocket;
  if(epoll_ctl(epoll, EPOLL_CTL_ADD, webSocket, &event) == -1) throw("ctl for webSocket failed!");

  event.data.fd = localSocket;
  if(epoll_ctl(epoll, EPOLL_CTL_ADD, localSocket, &event) == -1) throw("ctl for localSocket failed!");

  return epoll;
}

int getWebSocket(short portNum){
  struct sockaddr_in webAddress;
  webAddress.sin_family = AF_INET;
  webAddress.sin_port = htons(portNum);
  webAddress.sin_addr.s_addr = INADDR_ANY;

  int webSocket = socket(AF_INET, SOCK_DGRAM, 0);
  if(webSocket == -1) throw("Creating webSocket failed!");

  if(bind(webSocket, &webAddress, sizeof(webAddress)) == -1) throw("webSocket binding failed!");

  printf("Server address is %s\n", inet_ntoa(webAddress.sin_addr));

  return webSocket;
}

int getLocalSocket(char* path){
  struct sockaddr_un localAddress;
  localAddress.sun_family = AF_UNIX;
  for(int i=0; i<strlen(path) + 1; i++){
    localAddress.sun_path[i] = path[i];
  }

  int localSocket = socket(AF_UNIX, SOCK_DGRAM, 0);
  if(localSocket == -1) throw("Creating localSocket failed!");

  if(bind(localSocket, &localAddress, sizeof(localAddress)) == -1) throw("localSocket binding failed!");

  return localSocket;
}

short validatePort(short port){
  if((port < 1024) || (port > 60999)){
    throw("Port must be a number between 1024 and 60999!");
  }
  return port;
}

char* validatePath(char* path){
  int l = strlen(path);
  if((l < 1) || (l > UNIX_PATH_MAX)){
    printf("Path must be of length between 1 and %d\n", UNIX_PATH_MAX);
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
