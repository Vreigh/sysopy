#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#define MAX_CLIENTS  5
#define PROJECT_ID 37
#define MAX_CONT_SIZE 50

typedef enum mtype{
  LOGIN = 0, ECHO = 1, UPPER = 2, TIME = 3, END = 4, INIT = 5
} mtype;

typedef struct Msg{
  long mtype;
  pid_t senderPID;
  char cont[MAX_CONT_SIZE];
} Msg;

const size_t MSG_SIZE = sizeof(Msg) - sizeof(long);

#endif
