#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>

#include "helpers.h" // popFragment, trimWhite
#include "communication.h"

void publicQueueExecute(struct Msg* msg);
void doLogin(struct Msg* msg);
void doEcho(struct Msg* msg);
void doEchoUpper(struct Msg* msg);
void doTime(struct Msg* msg);
void doEnd(struct Msg* msg);
int findQID(pid_t senderPID);
int prepareMsg(struct Msg* msg);

int publicID = -2;
int active = 1;
int clientsData[MAX_CLIENTS][2];
int clientCnt = 0;

void rmQueue(void){
  if(publicID > -1){
    int tmp = msgctl(publicID, IPC_RMID, NULL);
    if(tmp == -1){
      printf("There was some error deleting server's queue!\n");
    }
    printf("Server's queue deleted successfully!\n");
  }
}

void intHandler(int signo){
  exit(2);
}

int main(int argc, char** argv){
  if(atexit(rmQueue) == -1) throw("Registering server's atexit failed!");
  if(signal(SIGINT, intHandler) == SIG_ERR) throw("Registering INT failed!");

  struct msqid_ds currentState;

  char* path = getenv("HOME");
  if(path == NULL) throw("Getting enviromental variable 'HOME' failed!");

  key_t publicKey = ftok(path, PROJECT_ID);
  if(publicKey == -1) throw("Generation of publicKey failed!");

  publicID = msgget(publicKey, IPC_CREAT | IPC_EXCL | 0666);
  if(publicID == -1) throw("Creation of public queue failed!");

  Msg buff;
  while(1){
    if(active == 0){
      if(msgctl(publicID, IPC_STAT, &currentState) == -1) throw("Getting current state of public queue failed!\n");
      if(currentState.msg_qnum == 0) break;
    }

    if(msgrcv(publicID, &buff, MSG_SIZE, 0, 0) < 0) throw("Receiving message failed!");
    publicQueueExecute(&buff);
  }
  return 0;
}

void publicQueueExecute(struct Msg* msg){
  if(msg == NULL) return;
  switch(msg->mtype){
    case LOGIN:
      doLogin(msg);
    break;
    case ECHO:
      doEcho(msg);
    break;
    case UPPER:
      doEchoUpper(msg);
    break;
    case TIME:
      doTime(msg);
    break;
    case END:
      doEnd(msg);
    break;
    default:
    break;
  }
}

void doLogin(struct Msg* msg){
  key_t clientQKey;
  if(sscanf(msg->cont, "%d", &clientQKey) < 0) throw("Reading clientKey failed!");

  int clientQID = msgget(clientQKey, 0);
  if(clientQID == -1 ) throw("Reading clientQID failed!");

  int clientPID = msg->senderPID;
  msg->mtype = INIT;
  msg->senderPID = getpid();

  if(clientCnt > MAX_CLIENTS - 1){
    printf("Maximum amount of clients reached!\n");
    sprintf(msg->cont, "%d", -1);
  }else{
    clientsData[clientCnt][0] = clientPID;
    clientsData[clientCnt++][1] = clientQID;
    sprintf(msg->cont, "%d", clientCnt-1);
  }

  if(msgsnd(clientQID, msg, MSG_SIZE, 0) == -1) throw("LOGIN response failed!");
}

void doEcho(struct Msg* msg){
  int clientQID = prepareMsg(msg);
  if(clientQID == -1) return;

  if(msgsnd(clientQID, msg, MSG_SIZE, 0) == -1) throw("ECHO response failed!");
}

void doEchoUpper(struct Msg* msg){
  int clientQID = prepareMsg(msg);
  if(clientQID == -1) return;

  for(int i=0; msg->cont[i] != '\0'; i++) msg->cont[i] = toupper(msg->cont[i]);

  if(msgsnd(clientQID, msg, MSG_SIZE, 0) == -1) throw("ECHO_UPPER response failed!");
}

void doTime(struct Msg* msg){
  int clientQID = prepareMsg(msg);
  if(clientQID == -1) return;

  time_t timer;
  time(&timer);
  char* timeStr = convertTime(&timer);

  sprintf(msg->cont, "%s", timeStr);
  free(timeStr);

  if(msgsnd(clientQID, msg, MSG_SIZE, 0) == -1) throw("TIME response failed!");
}

void doEnd(struct Msg* msg){
  active = 0;
}

int prepareMsg(struct Msg* msg){
  int clientQID = findQID(msg->senderPID);
  if(clientQID == -1){
    printf("Client Not Found!\n");
    return -1;
  }

  msg->mtype = msg->senderPID;
  msg->senderPID = getpid();

  return clientQID;
}

int findQID(pid_t senderPID){
  for(int i=0; i<MAX_CLIENTS; i++){
    if(clientsData[i][0] == senderPID) return clientsData[i][1];
  }
  return -1;
}
