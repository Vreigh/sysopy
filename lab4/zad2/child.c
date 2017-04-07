#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>

#include "helpers.h"

void usrHandler(int signo){
   kill(getppid(), SIGRTMIN + (rand() % (SIGRTMAX - SIGRTMIN)));
}

int main() {
  srand((unsigned int) getpid());
  signal(SIGUSR1, usrHandler);
  clock_t stop, start;

  printf("Hello World, Im %d, and im going to sleep...\n", getpid());
  fflush(stdout);
  sleep(rand() % 10);

  //printf("Hello Again! Im %d, and i just awoke!\n", getpid());
  //fflush(stdout);
  int wTime = 0;
  kill(getppid(), SIGUSR1);
  while(1){
    int tmp = sleep(100);
    if(tmp == 0) wTime += 100;
    else{
      wTime += (100 - tmp);
      break;
    }
  }

  printf("My parent noticed me, %d, im so excited!!\n", getpid());
  fflush(stdout);

  //double difReal = ((double)(stop - start)) / CLOCKS_PER_SEC;

  return wTime;
}
