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

#include "helpers.h"

void showUsage();
int validatePort(short port);
char* validatePath(char* path);
void quitter(int signo);
void cleanup(void);
int getWebSocket(short portNum);
int getLocalSocket(char* path);

int webSocket;
int localSocket;
char* unixPath;

int main(int argc, char** argv){
  if(argc != 3) showUsage();
  if(signal(SIGINT, quitter) == SIG_ERR) throw("Signal failed!");
  if(atexit(cleanup) != 0) throw("At exit failed!"); // procedura sprzatajaca

  short portNum = validatePort((short)atoi(argv[1])); // walidacja poprawnosci inputu
  unixPath = validatePath(argv[2]);

  webSocket = getWebSocket(portNum); // adres, socket i bindowanie
  localSocket = getLocalSocket(unixPath); // adres, socket i bindowanie

  sleep(100);
}

int getWebSocket(short portNum){
  struct sockaddr_in webAddress;
  webAddress.sin_family = AF_INET;
  webAddress.sin_port = htons(portNum);
  webAddress.sin_addr.s_addr = INADDR_ANY;

  int webSocket = socket(AF_INET, SOCK_STREAM, 0);
  if(webSocket == -1) throw("Creating webSocket failed!");

  if(bind(webSocket, &webAddress, sizeof(webAddress)) == -1) throw("Binding webSocket failed!");

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

  if(bind(localSocket, &localAddress, sizeof(localAddress)) == -1) throw("Binding localSocket failed!");

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

  if(allOk == 1) printf("All cleaned up!\n");
}

void quitter(int signo){
  exit(2);
}

void showUsage(){
  printf("Use program like <portNum> <unixPath>\n");
  exit(1);
}
