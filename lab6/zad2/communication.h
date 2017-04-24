#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#define MAX_CLIENTS  10
#define PROJECT_ID 37
#define MAX_CONT_SIZE 50

typedef enum mtype{
  LOGIN = 1, ECHO = 2, UPPER = 3, TIME = 4, END = 5, INIT = 6
} mtype;

typedef struct Msg{
  long mtype;
  pid_t senderPID;
  char cont[MAX_CONT_SIZE];
} Msg;

const size_t MSG_SIZE = sizeof(Msg);
const char* serverPath = "/server";

#endif
