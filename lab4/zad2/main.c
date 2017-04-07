#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#include "helpers.h"

volatile int N; // liczba potomkow do utworzenia
volatile int K; // liczba prosb do otrzymania
volatile int n; // obecna liczba potomkow
volatile int k; // ilosc prosb, oraz ilosc potomkow w kolejce
volatile pid_t* ascTab;
volatile pid_t* awaitingTab;

void usrHandler(int, siginfo_t*, void*);
void intHandler(int, siginfo_t*, void*);
void chldHandler(int, siginfo_t*, void*);
void rtHandler(int, siginfo_t*, void*);

void removeFromAsc(pid_t x){
  for(int i=0; i<N; i++){
    if(x == ascTab[i]){
      ascTab[i] = -1;
    }
  }
}

int checkIfInAsc(pid_t x){
  for(int i=0; i<N; i++){
    if(ascTab[i] == x) return 1;
  }
  return 0;
}

void showUsage(){
  printf("Use program like ./main.out <N> <K>\n");
  exit(1);
}

int main(int argc, char** argv){
  if(argc < 3) showUsage();
  N = atoi(argv[1]);
  K = atoi(argv[2]);
  if(N < 1 || N > 1000){
    printf("Wrong N!\n");
    return 1;
  }
  if(K < 1 || K > 1000){
    printf("Wrong N!\n");
    return 1;
  }
  if(K > N){
    printf("K cannot be larger than N! (I assume, that every child can send only one request)\n");
    return 2;
  }
  n = k = 0; // obecnie 0 prosb i 0 potomkow
  ascTab = calloc(N, sizeof(int));
  awaitingTab = calloc(K, sizeof(int));

  struct sigaction act; // rejestrowanie handlerow
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_SIGINFO;

  act.sa_sigaction = usrHandler;
  if(sigaction(SIGUSR1, &act, NULL) == -1){
    printf("Cannot catch SIGUSR1\n");
    return 1;
  }

  act.sa_sigaction = intHandler;
  if(sigaction(SIGINT, &act, NULL) == -1){
    printf("Cannot catch SIGINT\n");
    return 1;
  }

  //sigfillset(&act.sa_mask);
  act.sa_sigaction = chldHandler;
  if(sigaction(SIGCHLD, &act, NULL) == -1){
    printf("Cannot catch SIGCHLD\n");
    return 1;
  }

  //sigemptyset(&act.sa_mask);
  for(int i = SIGRTMIN; i <= SIGRTMAX; i++) {
    act.sa_sigaction = rtHandler;
    if(sigaction(i, &act, NULL) == -1){
      printf("Cannot catch i\n");
      return 1;
    }
  }

  for(int i=0; i<N; i++){
    int fork0 = fork();
    if(fork0 == 0){
      execl("./child.out", "./child.out", NULL);
      printf("Error creating child process, abording!\n");
      return 2;
    }else if(fork0 > 0){
      ascTab[i] = fork0;
      n++;
    }
  }
  while(wait(NULL)){
    if (errno == ECHILD){
        break;
    }
  }
  printf("ERROR CHILD %d!\n", n);
  return 2;
}

void usrHandler(int signo, siginfo_t* info, void* context){
  char buffer[100];
  sprintf(buffer, "MOTHERSHIP: Received SIGUSR1 from PID %d!\n", info->si_pid);
  write(1, buffer, strlen(buffer));

  if(checkIfInAsc(info->si_pid) != 1) return; // jezeli sygnal nie jest od dziecka to ignoruj

  if(k == K){
    kill(info->si_pid, SIGUSR1);
    waitpid(info->si_pid, NULL, 0); // chce obslugiwac jeden sygnal od dziecka naraz. Inaczej niektore by przepadly
  }
  else{
    awaitingTab[k++] = info->si_pid;
    if(k == K){
      for(int i=0; i<k; i++){
        kill(awaitingTab[i], SIGUSR1); // wysylam calej kolejce zgode na kontynuacje
        waitpid(awaitingTab[i], NULL, 0); // dziecko konczy sie zaraz po otrzymaniu zgody - czekajac na kazdego z osobna, moge
      } // kontrolowac ich liczbe. Inaczej, sygnaly by sie nie kolejkowaly, i czesc z nich by przepadla.
    }
  }
}

void intHandler(int signo, siginfo_t* info, void* context){
  char buffer[100];
  sprintf(buffer, "MOTHERSHIP: Received SIGINT from PID %d!\n", info->si_pid);
  write(1, buffer, strlen(buffer));

  for(int i=0; i<N; i++){
    if(ascTab[i] != -1){ // nie chce zabijac procesow, ktore juz nie sa moimi dziecmi
      kill(ascTab[i], SIGINT);
      waitpid(ascTab[i], NULL, 0);
    }
  }
  exit(0);
}

void chldHandler(int signo, siginfo_t* info, void* context){
  char buffer[100];
  sprintf(buffer, "MOTHERSHIP: Child Process %d has terminated, with exit status of: %d!\n", info->si_pid, info->si_status);
  write(1, buffer, strlen(buffer));
  n--;
  if(n == 0){
    sprintf(buffer, "MOTHERSHIP: All of my children are done, time for me :(\n");
    write(1, buffer, strlen(buffer));
    exit(0);
  }
  removeFromAsc(info->si_pid);
}

void rtHandler(int signo, siginfo_t* info, void* context){
  char buffer[100];
  sprintf(buffer, "MOTHERSHIP: Received real time signal: SIGMIN+%i from pid: %d\n", signo - SIGRTMIN, info->si_pid);
  write(1, buffer, strlen(buffer));
}
