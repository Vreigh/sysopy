#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <mqueue.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>

#include "helpers.h" // popFragment, trimWhite
#include "communication.h"

void publicQueueExecute(struct Msg* msg);
void doLogin(struct Msg* msg);
void doEcho(struct Msg* msg);
void doEchoUpper(struct Msg* msg);
void doTime(struct Msg* msg);
void doEnd(struct Msg* msg);
int findMQD(pid_t senderPID);
int prepareMsg(struct Msg* msg);

int active = 1;
int clientsData[MAX_CLIENTS][2];
int clientCnt = 0;
mqd_t publicID = -1;

void rmQueue(void){
  for(int i=0; i<clientCnt; i++){
    if(mq_close(clientsData[i][1]) == -1){
      printf("Error closing %d client queue\n", i);
    }
  }
  if(publicID > -1){
    if(mq_unlink(serverPath) == -1) printf("Error deleting public Queue!\n");
    else printf("Server queue deleted successfully!\n");
  }
}

int main(int argc, char** argv){
  if(atexit(rmQueue) == -1) throw("Registering server's atexit failed!");
  struct mq_attr currentState;

  publicID = mq_open(serverPath, O_RDONLY | O_CREAT | O_EXCL, 0666, NULL);
  if(publicID == -1) throw("Creation of public queue failed!");

  Msg buff;
  while(1){
    if(active == 0){
      if(mq_getattr(publicID, &currentState) == -1) throw("Couldnt read public queue parameters!");
      if(currentState.mq_curmsgs == 0) break;
    }

    if(mq_receive(publicID,(char*) &buff, MSG_SIZE, NULL) == -1) throw("Receiving message by server failed!");
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
  int clientPID = msg->senderPID;
  char clientPath[15];
  sprintf(clientPath, "/%d", clientPID);

  int clientMQD = mq_open(clientPath, O_WRONLY);
  if(clientMQD == -1 ) throw("Reading clientMQD failed!");

  msg->mtype = INIT;
  msg->senderPID = getpid();

  if(clientCnt > MAX_CLIENTS - 1){
    printf("Maximum amount of clients reached!\n");
    sprintf(msg->cont, "%d", -1);
    if(mq_send(clientMQD, (char*) msg, MSG_SIZE, 1) == -1) throw("Login response failed!");
    if(mq_close(clientMQD) == -1) throw("Closing client's queue failed!");
  }else{
    clientsData[clientCnt][0] = clientPID;
    clientsData[clientCnt++][1] = clientMQD;
    sprintf(msg->cont, "%d", clientCnt-1);
    if(mq_send(clientMQD, (char*) msg, MSG_SIZE, 1) == -1) throw("Login response failed!");
  }
}

void doEcho(struct Msg* msg){
  int clientMQD = prepareMsg(msg);
  if(clientMQD == -1) return;

  if(mq_send(clientMQD, (char*) msg, MSG_SIZE, 1) == -1) throw("ECHO response failed!");
}

void doEchoUpper(struct Msg* msg){
  int clientMQD = prepareMsg(msg);
  if(clientMQD == -1) return;

  for(int i=0; msg->cont[i] != '\0'; i++) msg->cont[i] = toupper(msg->cont[i]);

  if(mq_send(clientMQD, (char*) msg, MSG_SIZE, 1) == -1) throw("UPPER response failed!");
}

void doTime(struct Msg* msg){
  int clientMQD = prepareMsg(msg);
  if(clientMQD == -1) return;

  time_t timer;
  time(&timer);
  char* timeStr = convertTime(&timer);

  sprintf(msg->cont, "%s", timeStr);
  free(timeStr);

  if(mq_send(clientMQD, (char*) msg, MSG_SIZE, 1) == -1) throw("TIME response failed!");
}

void doEnd(struct Msg* msg){
  active = 0;
}

int prepareMsg(struct Msg* msg){
  int clientMQD = findMQD(msg->senderPID);
  if(clientMQD == -1){
    printf("Client Not Found!\n");
    return -1;
  }

  msg->mtype = msg->senderPID;
  msg->senderPID = getpid();

  return clientMQD;
}

int findMQD(pid_t senderPID){
  for(int i=0; i<MAX_CLIENTS; i++){
    if(clientsData[i][0] == senderPID) return clientsData[i][1];
  }
  return -1;
}
