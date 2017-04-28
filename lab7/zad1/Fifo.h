#ifndef FIFO_H
#define FIFO_H

typedef struct Fifo{
  int max;
  int head;
  int tail;
  pid_t chair;
  pid_t tab[1000];
} Fifo;

void fifoInit(Fifo* fifo, int cn);

#endif
