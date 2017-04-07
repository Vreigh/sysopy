#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#include "helpers.h"


int SLP = 300000;
volatile int choice = 1;

void stpHandler(int signo, siginfo_t* info, void* context){
  if(choice == 1) choice = 0;
  else choice = 1;

  struct sigaction act;
  act.sa_flags = SA_SIGINFO;
  sigemptyset(&act.sa_mask);
  act.sa_sigaction = stpHandler;
  sigaction(SIGTSTP, &act, NULL);
}

void intHandler(int signo){
  printf("\nIm %d And Im Killing Myself. \n", getpid());
  exit(0);
}

int main(){
  if (signal(SIGINT, intHandler) == SIG_ERR){
    printf("\ncan't catch SIGINT\n");
    return 1;
  }

  struct sigaction act;
  act.sa_flags = SA_SIGINFO;
  sigemptyset(&act.sa_mask);
  act.sa_sigaction = stpHandler;
  if(sigaction(SIGTSTP, &act, NULL) == -1){
    printf("\ncan't catch SIGTSTP\n");
    return 2;
  }

  int i = 64;
  while(1){
    if(choice == 1){
      if(i == 90){
        printf("\n");
        i = 65;
      }else i++;
    }else{
      if(i == 65){
        printf("\n");
        i = 90;
      }else i--;
    }
    if(i < 65) i = 65;
    char out = i;
    printf("%c ", out);
    fflush(stdout);
    usleep(SLP);
  }
  return 0;
}
