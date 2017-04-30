#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/msg.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

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

Fifo* fifo = NULL;

sem_t* BARBER;
sem_t* FIFO;
sem_t* CHECKER;
sem_t* SLOWER;

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
    if(sem_wait(CHECKER) == -1) throw("Client: taking checker failed!");

    if(sem_wait(FIFO) == -1) throw("Client: taking FIFO failed!");

    int res = takePlace();

    if(sem_post(FIFO) == -1) throw("Client: releasing FIFO failed!");

    if(sem_post(CHECKER) == -1) throw("Client: releasing checker failed!");

    if(res != -1){
      sigsuspend(&fullMask);
      long timeMarker = getMicroTime();
      printf("Time: %ld, Client %d just got cut!\n", timeMarker, getpid()); fflush(stdout);
    }
  }
}

int takePlace(){
  int barberStat;
  if(sem_getvalue(BARBER, &barberStat) == -1) throw("Client: getting value of BARBER sem failed!");

  pid_t myPID = getpid();

  if(barberStat == 0){
    if(sem_post(BARBER) == -1) throw("Client: awakening barber failed!");
    long timeMarker = getMicroTime();
    printf("Time: %ld, Client %d has awakened barber!\n", timeMarker, myPID); fflush(stdout);
    if(sem_wait(SLOWER) == -1) throw("Client: waiting for barber failed!"); // waiting for barber to set his value to 1

    fifo->chair = myPID;

    return 1;
  }else{
    int res =  pushFifo(fifo, myPID);
    if(res == -1){
      long timeMarker = getMicroTime();
      printf("Time: %ld, Client %d couldnt find free place!\n", timeMarker, myPID); fflush(stdout);
      return -1;
    }else{
      long timeMarker = getMicroTime();
      printf("Time: %ld, Client %d took place in the queue!\n", timeMarker, myPID); fflush(stdout);
      return 0;
    }
  }
}

void prepareFifo(){
  int shmID = shm_open(shmPath, O_RDWR, 0666);
  if(shmID == -1) throw("Breeder: opening shared memory failed!");

  //if(ftruncate(shmID, sizeof(Fifo)) == -1) throw("Breeder: truncating shm failed!");

  void* tmp = mmap(NULL, sizeof(Fifo), PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
  if(tmp == (void*)(-1)) throw("Breeder: attaching shm failed!");
  fifo = (Fifo*) tmp;
}

void prepareSemafors(){
  BARBER = sem_open(barberPath, O_RDWR);
  if(BARBER == SEM_FAILED) throw("Breeder: creating semafors failed!");

  FIFO = sem_open(fifoPath, O_RDWR);
  if(FIFO == SEM_FAILED) throw("Breeder: creating semafors failed!");

  CHECKER = sem_open(checkerPath, O_RDWR);
  if(CHECKER == SEM_FAILED) throw("Breeder: creating semafors failed!");

  SLOWER = sem_open(slowerPath, O_RDWR);
  if(SLOWER == SEM_FAILED) throw("Breeder: creating semafors failed!");
}

void prepareFullMask(){
  if(sigfillset(&fullMask) == -1) throw("Breeder: sigfillset failed!");
  if(sigdelset(&fullMask, SIGRTMIN) == -1) throw("Breeder: removing sigrtmin from fullMask failed!");
  if(sigdelset(&fullMask, SIGINT) == -1) throw("Breeder: removing sigint from fullMask failed!");
}

void freeResources(void){
  if(munmap(fifo, sizeof(fifo)) == -1) printf("Breeder: Error detaching fifo sm!\n");
  else printf("Breeder: detached fifo sm!\n");

  if(sem_close(BARBER) == -1) printf("Barber: Error closing semafors!");
  if(sem_close(FIFO) == -1) printf("Barber: Error closing semafors!");
  if(sem_close(CHECKER) == -1) printf("Barber: Error closing semafors!");
  if(sem_close(SLOWER) == -1) printf("Barber: Error closing semafors!");

  printf(" OK!");
}
