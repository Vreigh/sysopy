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

#include "helpers.h" // popFragment, trimWhite, getQID
#include "communication.h"

void registerClient();
void rqEcho(struct Msg *msg);
void rqEchoUpper(struct Msg *msg);
void rqTime(struct Msg *msg);
void rqEnd(struct Msg *msg);

int sessionID = -2;
mqd_t publicID = -1;
mqd_t privateID = -1;
char myPath[20];

void rmQueue(void){
  if(privateID > -1){
    if(mq_unlink(myPath) == -1){
      printf("There was some error deleting server's queue!\n");
    }
    else printf("Client's queue deleted successfully!\n");
  }
}

int main(int argc, char** argv){
  if(atexit(rmQueue) == -1) throw("Registering client's atexit failed!");

  sprintf(myPath, "/%d", getpid());

  publicID = mq_open(serverPath, O_WRONLY);
  if(publicID == -1) throw("Opening public queue failed!");

  privateID = mq_open(myPath, O_RDONLY | O_CREAT | O_EXCL, 0666, NULL);
  if(privateID == -1) throw("Creation of private queue failed!");

  registerClient();

  char cmd[20];
  Msg msg;
  while(1){
    msg.senderPID = getpid();
    printf("Enter your request: ");
    if(fgets(cmd, 20, stdin) == NULL){
      printf("Error reading your command!\n");
      continue;
    }
    int n = strlen(cmd);
    if(cmd[n-1] == '\n') cmd[n-1] = 0;


    if(strcmp(cmd, "echo") == 0){
      rqEcho(&msg);
    }else if(strcmp(cmd, "upper") == 0){
      rqEchoUpper(&msg);
    }else if(strcmp(cmd, "time") == 0){
      rqTime(&msg);
    }else if(strcmp(cmd, "end") == 0){
      rqEnd(&msg);
    }else if(strcmp(cmd, "q") == 0){
      break;
    }else printf("Wrong command!\n");
  }
  return 0;
}

void registerClient(){
  Msg msg;
  msg.mtype = LOGIN;
  msg.senderPID = getpid();

  if(mq_send(publicID, (char*) &msg, MSG_SIZE, 1) == -1) throw("Login request failed!");
  if(mq_receive(privateID,(char*) &msg, MSG_SIZE, NULL) == -1) throw("Catching LOGIN response failed!");
  if(sscanf(msg.cont, "%d", &sessionID) < 1) throw("scanning LOGIN response failed!");
  if(sessionID < 0) throw("Server cannot have more clients!");

  printf("Client registered! My session nr is %d!\n", sessionID);
}

void rqEcho(struct Msg *msg){
  msg->mtype = ECHO;
  if(fgets(msg->cont, MAX_CONT_SIZE, stdin) == NULL){
    printf("Too many characters!\n");
    return;
  }
  if(mq_send(publicID, (char*) msg, MSG_SIZE, 1) == -1) throw("ECHO request failed!");
  if(mq_receive(privateID,(char*) msg, MSG_SIZE, NULL) == -1) throw("Catching ECHO response failed!");
  printf("%s", msg->cont);
}
void rqEchoUpper(struct Msg *msg){
  msg->mtype = UPPER;
  if(fgets(msg->cont, MAX_CONT_SIZE, stdin) == NULL){
    printf("Too many characters!\n");
    return;
  }
  if(mq_send(publicID, (char*) msg, MSG_SIZE, 1) == -1) throw("UPPER request failed!");
  if(mq_receive(privateID,(char*) msg, MSG_SIZE, NULL) == -1) throw("Catching UPPER response failed!");
  printf("%s", msg->cont);
}
void rqTime(struct Msg *msg){
  msg->mtype = TIME;

  if(mq_send(publicID, (char*) msg, MSG_SIZE, 1) == -1) throw("TIME request failed!");
  if(mq_receive(privateID,(char*) msg, MSG_SIZE, NULL) == -1) throw("Catching TIME response failed!");
  printf("%s\n", msg->cont);
}
void rqEnd(struct Msg *msg){
  msg->mtype = END;

  if(mq_send(publicID, (char*) msg, MSG_SIZE, 1) == -1) throw("END request failed!");
}
