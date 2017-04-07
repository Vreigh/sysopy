#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#include "helpers.h"
volatile int type;
volatile int byChildCnt = 0;
volatile int fromChildCnt = 0;
volatile pid_t f;
int SLP;
void showFinalInfo();

void showUsage(){
  printf("Use program like: ./main.out <L> <Type>\n");
  exit(1);
}
void checkType(){
  if(type > 3 || type < 1){
    printf("Wrong Type Argument!\n");
    exit(2);
  }
}
void checkL(int L){
  if(L > 1000000 || L < 1){
    printf("Wrong L Argument!\n");
    exit(3);
  }
}
void checkSLP(){
  if(SLP < 0 || SLP > 10000){
    printf("Wrong SLP Argument!\n");
    exit(7);
  }
}

void childHandler(int signo, siginfo_t* info, void* context){
  char buffer[100];
  if(signo == SIGINT){
    sprintf(buffer, "Signals received by child: %d\n", byChildCnt);
    write(1, buffer, strlen(buffer));
    exit((unsigned)byChildCnt); // zadziala poprawnie tylko do 2 do 8!!
  }


  if(info->si_pid == getppid()){
    if(type == 1 || type == 2){
      if(signo == SIGUSR1){
        byChildCnt++;
        kill(info->si_pid, SIGUSR1);
        sprintf(buffer, "CHILD: SIGUSR1 taken and sent back!\n");
        write(1, buffer, strlen(buffer));
      }
      else if(signo == SIGUSR2){
        byChildCnt++;
        sprintf(buffer, "CHILD: SIGUSR2 taken, abording!\n");
        write(1, buffer, strlen(buffer));
        sprintf(buffer, "Signals received by child: %d\n", byChildCnt);
        write(1, buffer, strlen(buffer));
        exit((unsigned)byChildCnt); // zadziala poprawnie tylko do 2 do 8!!
      }
    }
    else if(type == 3){
      if(signo == SIGRTMIN){
        byChildCnt++;
        kill(info->si_pid, SIGRTMIN);
        sprintf(buffer, "CHILD: SIGRTMIN taken and sent back!\n");
        write(1, buffer, strlen(buffer));
      }
      else if(signo == SIGRTMAX){
        byChildCnt++;
        sprintf(buffer, "CHILD: SIGRTMAX taken, abording!\n");
        write(1, buffer, strlen(buffer));
        sprintf(buffer, "Signals received by child: %d\n", byChildCnt);
        write(1, buffer, strlen(buffer));
        exit((unsigned)byChildCnt); //zadziala poprawnie tylko do 2 do 8!!
      }
    }
  }
  return; // w innych przypadkach ignoruj ten sygnal
}
void parentHandler(int signo, siginfo_t* info, void* context){
  if(signo == SIGINT){
    kill(f, SIGINT);
    return; // wroc do parent act, ktore natychmiast sie skonczy, wroci do main i wypisze info
  }
  char buffer[100];
  if(info->si_pid == f){
    if(type == 1 || type == 2){
      if(signo == SIGUSR1){
        fromChildCnt++;
        sprintf(buffer, "PARENT: Received SIGUSR1 From Child!\n");
        write(1, buffer, strlen(buffer));
      }

    }else if(type == 3){
      if(signo == SIGRTMIN){
        fromChildCnt++;
        sprintf(buffer, "PARENT: Received SIGRTMIN From Child!\n");
        write(1, buffer, strlen(buffer));
      }
    }
  }
  return; // inne sygnaly ignoruj
}

void childAct(){
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = childHandler;
  if(sigaction(SIGINT, &act, NULL)== -1){
    printf("Error setting SIGINT handler");
    exit(6);
  }
  if(sigaction(SIGUSR1, &act, NULL)== -1){
    printf("Error setting SIGUSR1 handler");
    exit(6);
  }
  if(sigaction(SIGUSR2, &act, NULL)== -1){
    printf("Error setting SIGUSR2 handler");
    exit(6);
  }
  if(sigaction(SIGRTMIN, &act, NULL)== -1){
    printf("Error setting SIGRTMIN handler");
    exit(6);
  }
  if(sigaction(SIGRTMAX, &act, NULL)== -1){
    printf("Error setting SIGRTMAX handler");
    exit(6);
  }

  while(1){
    sleep(1);
  }
}

void parentAct(int L){
  sleep(1); // zeby dziecko zdazylo sie przygotowac
  struct sigaction act; // rejestrowanie handlerow
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = parentHandler;
  if(sigaction(SIGINT, &act, NULL)== -1){
    printf("Error setting SIGINT handler");
    exit(5);
  }
  if(sigaction(SIGUSR1, &act, NULL)== -1){
    printf("Error setting SIGUSR1 handler");
    exit(5);
  }
  if(sigaction(SIGRTMIN, &act, NULL)== -1){
    printf("Error setting SIGRTMIN handler");
    exit(5);
  }

  if(type == 1){
    for(int i=0; i<L; i++){
      printf("PARENT: Sending SIGUSR1 signal!\n");
      kill(f, SIGUSR1);
      //usleep(SLP);
    }
    printf("PARENT: Sending SIGUSR2 signal!\n");
    kill(f, SIGUSR2);
  }else if(type ==2){
    union sigval value;
    for(int i=0; i<L; i++){
      printf("PARENT: Sending SIGUSR1 signal!\n");
      sigqueue(f, SIGUSR1, value);
      //usleep(SLP);
    }
    printf("PARENT: Sending SIGUSR2 signal!\n");
    sigqueue(f, SIGUSR2, value);
  }else if(type == 3){
    for(int i=0; i<L; i++){
      printf("PARENT: Sending SIGRTMIN signal!\n");
      kill(f, SIGRTMIN);
      //usleep(SLP);
    }
    printf("PARENT: Sending SIGRTMAX signal!\n");
    kill(f, SIGRTMAX);
  }

  int status;
  waitpid(f, &status, 0);
  if(WIFEXITED(status)){
    byChildCnt = WEXITSTATUS(status); // poprawnie tylko do 2 do 8!!
  }else{
    printf("Error with termination of Child!\n");
    exit(8);
  }
}

int main(int argc, char** argv){
  if(argc < 3) showUsage();
  int L = atoi(argv[1]);
  type = atoi(argv[2]);
  //SLP = atoi(argv[3]);
  checkType();
  checkL(L);
  //checkSLP();
  SLP *= 1000; // z ms

  f = fork();
  if(f == 0) childAct();
  else if(f > 0) parentAct(L);
  else return 4;

  showFinalInfo(L);
  return 0;
}

void showFinalInfo(int L){
  printf("Signals sent: %d\n", L+1);
  printf("Signals received from child: %d\n", fromChildCnt+1);
}
