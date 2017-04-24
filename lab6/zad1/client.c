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

#include "helpers.h" // popFragment, trimWhite, getQID
#include "communication.h"

void registerClient(key_t privateKey);
void rqEcho(struct Msg *msg);
void rqEchoUpper(struct Msg *msg);
void rqTime(struct Msg *msg);
void rqEnd(struct Msg *msg);

int sessionID = -2;
int publicID = -1;
int privateID = -1;

void rmQueue(void){
  if(privateID > -1){
    if(msgctl(privateID, IPC_RMID, NULL) == -1){
      printf("There was some error deleting server's queue!\n");
    }
    else printf("Client's queue deleted successfully!\n");
  }
}

int main(int argc, char** argv){
  if(atexit(rmQueue) == -1) throw("Registering client's atexit failed!");

  char* path = getenv("HOME");
  if(path == NULL) throw("Getting enviromental variable 'HOME' failed!");

  publicID = getQID(path, PROJECT_ID);

  key_t privateKey = ftok(path, getpid());
  if(privateKey == -1) throw("Generation of private key failed!");

  privateID = msgget(privateKey, IPC_CREAT | IPC_EXCL | 0666);
  if(privateID == -1) throw("Creation of private queue failed!");

  registerClient(privateKey);

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

void registerClient(key_t privateKey){
  Msg msg;
  msg.mtype = LOGIN;
  msg.senderPID = getpid();
  sprintf(msg.cont, "%d", privateKey);

  if(msgsnd(publicID, &msg, MSG_SIZE, 0) == -1) throw("LOGIN request failed!");
  if(msgrcv(privateID, &msg, MSG_SIZE, 0, 0) == -1) throw("catching LOGIN response failed!");
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
  if(msgsnd(publicID, msg, MSG_SIZE, 0) == -1) throw("ECHO request failed!");
  if(msgrcv(privateID, msg, MSG_SIZE, 0, 0) == -1) throw("catching ECHO response failed!");
  printf("%s", msg->cont);
}
void rqEchoUpper(struct Msg *msg){
  msg->mtype = UPPER;
  if(fgets(msg->cont, MAX_CONT_SIZE, stdin) == NULL){
    printf("Too many characters!\n");
    return;
  }
  if(msgsnd(publicID, msg, MSG_SIZE, 0) == -1) throw("UPPER request failed!");
  if(msgrcv(privateID, msg, MSG_SIZE, 0, 0) == -1) throw("catching UPPER response failed!");
  printf("%s", msg->cont);
}
void rqTime(struct Msg *msg){
  msg->mtype = TIME;

  if(msgsnd(publicID, msg, MSG_SIZE, 0) == -1) throw("TIME request failed!");
  if(msgrcv(privateID, msg, MSG_SIZE, 0, 0) == -1) throw("catching TIME response failed!");
  printf("%s\n", msg->cont);
}
void rqEnd(struct Msg *msg){
  msg->mtype = END;

  if(msgsnd(publicID, msg, MSG_SIZE, 0) == -1) throw("END request failed!");
}
