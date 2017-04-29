#include <stdlib.h>
#include "Fifo.h"

void fifoInit(Fifo* fifo, int cn){
  fifo->max = cn;
  fifo->head = -1;
  fifo->tail = 0;
  fifo->chair = 0;
}

int isEmptyFifo(Fifo* fifo){
  if(fifo->head == -1) return 1;
  else return 0;
}

int isFullFifo(Fifo* fifo){
  if(fifo->head == fifo->tail) return 1;
  else return 0;
}

pid_t popFifo(Fifo* fifo){
  if(isEmptyFifo(fifo) == 1) return -1;

  fifo->chair = fifo->tab[fifo->head++];
  if(fifo->head == fifo->max) fifo->head = 0;

  if(fifo->head == fifo->tail) fifo->head = -1;

  return fifo->tab[fifo->head];
}

int pushFifo(Fifo* fifo, pid_t x){
  if(isFullFifo(fifo) == 1) return 0;

  fifo->tab[fifo->tail++] = x;
  if(fifo->tail == fifo->max) fifo->tail = 0;
  return 1;
}
