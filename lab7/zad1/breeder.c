#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "helpers.h" // popFragment, trimWhite, getQID
#include "Fifo.h"
#include "hairdresser.h"

void showUsage(){
  printf("Use program like ./breeder.out <clientsNumber> <cutsNumber>\n");
  exit(1);
}
int validateClNum(int num){
  if(num < 1 || num > 500){
    throw("Wrong number of Clients!");
    return -1;
  }
  else return num;
}
int validateCtsNum(int num){
  if(num < 1 || num > 15){
    throw("Wrong number of Cuts!");
    return -1;
  }
  else return num;
}
void intHandler(int signo){
  exit(2);
}

void prepareFifo();
void prepareSemafors();
void prepareFullMask();
void freeResources(void);
int takePlace();
void getCut(int ctsNum);

key_t fifoKey;
int shmID = -1;
Fifo* fifo = NULL;
int SID = -1;

volatile int ctsCounter = 0;
sigset_t fullMask;

void rtminHandler(int signo){
  ctsCounter++;
}

int main(int argc, char** argv){
  if(argc != 3) showUsage();
  if(atexit(freeResources) == -1) throw("Breeder: atexit failed!");
  if(signal(SIGINT, intHandler) == SIG_ERR) throw("Breeder: signal failed!");
  if(signal(SIGRTMIN, rtminHandler) == SIG_ERR) throw("Breeder: signal failed!");

  int clNum = validateClNum(atoi(argv[1]));
  int ctsNum = validateCtsNum(atoi(argv[2]));

  prepareFifo();
  prepareSemafors();
  prepareFullMask();

  sigset_t mask;
  if(sigemptyset(&mask) == -1) throw("Breeder: emptyset failed!");
  if(sigaddset(&mask, SIGRTMIN) == -1) throw("Breeder: sigaddset failed!");
  if(sigprocmask(SIG_BLOCK, &mask, NULL) == -1) throw("Breeder: sigprocmask failed!");

  for(int i=0; i<clNum; i++){
    pid_t id = fork();
    if(id == -1) throw("Fork failed...");
    if(id == 0){
      getCut(ctsNum);
      return 0;
    }
  }

  printf("All clients has been bred!\n");
  while(1){
    wait(NULL); // czekaj na dzieci
    if (errno == ECHILD) break;
  }

  return 0;
}
void getCut(int ctsNum){
  while(ctsCounter < ctsNum){
    // wez checker
    // wez kolejke
    int res = takePlace();
    // zwolnij kolejke
    // zwolnij checker
    if(res != -1){
      sigsuspend(&fullMask);
    }
  }
}

int takePlace(){
  // barber spi - usiadz na krzesle, 2x go obudz, return 1
  // barber pracuje - zajmij miejsce, return odpowiednia wartosc (-1 lub 0)
}

void prepareFifo(){
  char* path = getenv(env);
  if(path == NULL) throw("Breeder: Getting enviromental variable failed!");

  fifoKey = ftok(path, PROJECT_ID);
  if(fifoKey == -1) throw("Breeder: getting key of shm failed!");

  shmID = shmget(fifoKey, 0, 0);
  if(shmID == -1) throw("Breeder: opening shm failed!");

  void* tmp = (Fifo*) shmat(shmID, NULL, 0);
  if(tmp == (void*)(-1)) throw("Breeder: attaching shm failed!");
  fifo = (Fifo*) tmp;
}

void prepareSemafors(){
  SID = semget(fifoKey, 0, 0);
  if(SID == -1) throw("Breeder: opening semafors failed!");
}

void prepareFullMask(){
  if(sigfillset(&fullMask) == -1) throw("Breeder: sigfillset failed!");
  if(sigdelset(&fullMask, SIGRTMIN) == -1) throw("Breeder: removing sigrtmin from fullMask failed!");
  if(sigdelset(&fullMask, SIGINT) == -1) throw("Breeder: removing sigint from fullMask failed!");
}

void freeResources(void){
  if(shmdt(fifo) == -1) printf("Breeder: Error detaching fifo sm!\n");
  else printf("Breeder: detached fifo sm!\n");
}
