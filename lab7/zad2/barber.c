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

#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "helpers.h" // popFragment, trimWhite, getQID
#include "Fifo.h"
#include "hairdresser.h"

void showUsage(){
  printf("Use program like ./barber.out <chairNumber>\n");
  exit(1);
}
void validateChNum(int num){
  if(num < 5 || num > 1000) throw("Wrong number of Chairs!");
}
void intHandler(int signo){
  exit(2);
}

void clearResources(void);
void prepareFifo(int chNum);
void prepareSemafors();
void napAndWorkForever();
void cut(pid_t pid);
pid_t takeChair();

Fifo* fifo = NULL;

sem_t* BARBER;
sem_t* FIFO;
sem_t* CHECKER;
sem_t* SLOWER;

int main(int argc, char** argv){
  if(argc != 2) showUsage();
  if(atexit(clearResources) == -1) throw("Barber: atexit failed!");
  if(signal(SIGINT, intHandler) == SIG_ERR) throw("Barber: signal failed!");

  prepareFifo(atoi(argv[1]));
  prepareSemafors();
  napAndWorkForever();

  return 0;
}

void napAndWorkForever(){
  while(1){
    if(sem_wait(BARBER) == -1) throw("Barber: 0 sops failed!"); // czekaj na obudzenie

    if(sem_post(BARBER) == -1) throw("Barber: setting himself as awaken failed!");
    if(sem_post(SLOWER) == -1) throw("Barber: freeing client failed!");

    pid_t toCut = takeChair();
    cut(toCut);

    while(1){
      if(sem_wait(FIFO) == -1) throw("Barber: 3 sops failed!");
      toCut = popFifo(fifo); // zajmij FIFO i pobierz pierwszego z kolejki

      if(toCut != -1){ // jesli istnial, to zwolnij kolejke, ostrzyz i kontynuuj
        if(sem_post(FIFO) == -1) throw("Barber: 4 sops failed!");
        cut(toCut);
      }else{ // jesli kolejka pusta, to ustaw, ze spisz, zwolnij kolejke i spij dalej (wyjdz z petli)
        long timeMarker = getMicroTime();
        printf("Time: %ld, Barber: going to sleep...\n", timeMarker);  fflush(stdout);

        if(sem_wait(BARBER) == -1) throw("Barber: 5 sops failed!");

        if(sem_post(FIFO) == -1) throw("Barber: 6 sops failed!");
        break;
      }
    }
  }
}

pid_t takeChair(){
  if(sem_wait(FIFO) == -1) throw("Barber: 1 sem failed!");

  pid_t toCut = fifo->chair;

  if(sem_post(FIFO) == -1) throw("Barber: 2 sem failed!");

  return toCut;
}

void cut(pid_t pid){
  long timeMarker = getMicroTime();
  printf("Time: %ld, Barber: preparing to cut %d\n", timeMarker, pid); fflush(stdout);

  kill(pid, SIGRTMIN);
  //if(sem_post(WAITER) == -1) throw("Barber: cutting failed!")

  timeMarker = getMicroTime();
  printf("Time: %ld, Barber: finished cutting %d\n", timeMarker, pid); fflush(stdout);
}

void prepareFifo(int chNum){
  validateChNum(chNum);

  int shmID = shm_open(shmPath, O_CREAT | O_EXCL | O_RDWR, 0666);
  if(shmID == -1) throw("Barber: creating shared memory failed!");

  if(ftruncate(shmID, sizeof(Fifo)) == -1) throw("Barber: truncating shm failed!");

  void* tmp = mmap(NULL, sizeof(Fifo), PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
  if(tmp == (void*)(-1)) throw("Barber: attaching shm failed!");
  fifo = (Fifo*) tmp;

  fifoInit(fifo, chNum);
}

void prepareSemafors(){
  BARBER = sem_open(barberPath, O_CREAT | O_EXCL | O_RDWR, 0666, 0);
  if(BARBER == SEM_FAILED) throw("Barber: creating semafors failed!");

  FIFO = sem_open(fifoPath, O_CREAT | O_EXCL | O_RDWR, 0666, 1);
  if(FIFO == SEM_FAILED) throw("Barber: creating semafors failed!");

  CHECKER = sem_open(checkerPath, O_CREAT | O_EXCL | O_RDWR, 0666, 1);
  if(CHECKER == SEM_FAILED) throw("Barber: creating semafors failed!");

  SLOWER = sem_open(slowerPath, O_CREAT | O_EXCL | O_RDWR, 0666, 0);
  if(SLOWER == SEM_FAILED) throw("Barber: creating semafors failed!");
}

void clearResources(void){
  if(munmap(fifo, sizeof(fifo)) == -1) printf("Barber: Error detaching fifo sm!\n");
  else printf("Barber: detached fifo sm!\n");

  if(shm_unlink(shmPath) == -1) printf("Barber: Error deleting fifo sm!\n");
  else printf("Barber: deleted fifo sm!\n");

  if(sem_close(BARBER) == -1) printf("Barber: Error closing semafors!");
  if(sem_unlink(barberPath) == -1) printf("Barber: Error deleting semafors!");

  if(sem_close(FIFO) == -1) printf("Barber: Error closing semafors!");
  if(sem_unlink(fifoPath) == -1) printf("Barber: Error deleting semafors!");

  if(sem_close(CHECKER) == -1) printf("Barber: Error closing semafors!");
  if(sem_unlink(checkerPath) == -1) printf("Barber: Error deleting semafors!");

  if(sem_close(SLOWER) == -1) printf("Barber: Error closing semafors!");
  if(sem_unlink(slowerPath) == -1) printf("Barber: Error deleting semafors!");

  printf("If no errors are shown, all semaphors has been successfully closed and unlinked!\n");
}
