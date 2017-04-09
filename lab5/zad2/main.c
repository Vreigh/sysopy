#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <complex.h>

#include "helpers.h"

volatile int S;
volatile pid_t masterCp;
volatile int fifoReady = 0;

void showUsage(){
  printf("Use program like ./main.out <path> <S> <N> <K> <R>\n");
  exit(1);
}

void rtHandler(int signo){
  if(signo == SIGRTMIN){
    if(--S == 0) kill(masterCp, SIGRTMIN);
  }
  else if(signo == SIGRTMIN+1) fifoReady++;
  else if(signo == SIGRTMIN+2){
    write(1, "ABORDING!\n", 10);
    exit(2);
  }
}

int main(int argc, char** argv){
  if(argc != 6) showUsage();
  S = atoi(argv[2]);
  int N = atoi(argv[3]);
  int K = atoi(argv[4]);
  int R = atoi(argv[5]);

  if(S < 1 || S > 200){
    printf("Wrong S!\n");
    return 1;
  }
  if(N > 1000000000 || N < 10){
    printf("Wrong N!\n");
    return 1;
  }
  if(K < 0 || K > 10000){
    printf("Wrong K!\n");
    return 1;
  }
  if(R < 50 || R > 1000){
    printf("Wrong R!\n");
    return 1;
  }

  if(signal(SIGRTMIN, rtHandler) == SIG_ERR){
    printf("Main couldnt set handler!\n");
    return 3;
  }
  if(signal(SIGRTMIN+1, rtHandler) == SIG_ERR){
    printf("Main couldnt set handler!\n");
    return 3;
  }
  if(signal(SIGRTMIN+2, rtHandler) == SIG_ERR){
    printf("Main couldnt set handler!\n");
    return 3;
  }

  char* masterArgv[4];
  masterArgv[0] = "./master.out";
  masterArgv[1] = argv[1];
  masterArgv[2] = argv[5];
  masterArgv[3] = NULL;
  masterCp = fork();
  if(masterCp == -1){
    printf("Error, couldnt fork master!\n");
    return 2;
  }else if(masterCp == 0){
    execvp(masterArgv[0], masterArgv);
    printf("MASTER WASNT CREATED!\n");
    return 3;
  }

  while(fifoReady != 1){
    sleep(1);
  }
  printf("I, main, received signal, that FIFO is created!\n"); fflush(stdout);

  char* slaveArgv[5];
  slaveArgv[0] = "./slave.out";
  slaveArgv[1] = argv[1];
  slaveArgv[2] = argv[3];
  slaveArgv[3] = argv[4];
  slaveArgv[4] = NULL;

  int slaveNmb = S;

  for(int i=0; i<slaveNmb; i++){
    pid_t cp = fork();
    if(cp == -1){
      printf("Error, couldnt fork slave!\n");
      return 2;
    }else if(cp == 0){
      execvp(slaveArgv[0], slaveArgv);
      printf("SLAVE WASNT CREATED!\n");
      return 3;
    }
  }

  printf("All programs are started at this point!\n");
  while(1){
    wait(NULL); // czekaj na dzieci
    if (errno == ECHILD) break;
  }
  printf("All programs are finished at this point!\n");
  return 0;
}
