#include <stdlib.h>
#include "Fifo.h"

void fifoInit(Fifo* fifo, int cn){
  fifo->max = cn;
  fifo->head = -1;
  fifo->tail = 0;
  fifo->chair = 0;
}
