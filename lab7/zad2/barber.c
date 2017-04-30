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
pid_t takeChair(struct sembuf*);

key_t fifoKey;
int shmID = -1;
Fifo* fifo = NULL;
int SID = -1;

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
    struct sembuf sops;
    sops.sem_num = BARBER;
    sops.sem_op = -1;
    sops.sem_flg = 0;

    if(semop(SID, &sops, 1) == -1) throw("Barber: 0 sops failed!"); // czekaj na obudzenie

    pid_t toCut = takeChair(&sops);
    cut(toCut);

    while(1){
      sops.sem_num = FIFO; // dla czytelnosci
      sops.sem_op = -1;
      if(semop(SID, &sops, 1) == -1) throw("Barber: 3 sops failed!");
      toCut = popFifo(fifo); // zajmij FIFO i pobierz pierwszego z kolejki

      if(toCut != -1){ // jesli istnial, to zwolnij kolejke, ostrzyz i kontynuuj
        sops.sem_op = 1;
        if(semop(SID, &sops, 1) == -1) throw("Barber: 4 sops failed!");
        cut(toCut);
      }else{ // jesli kolejka pusta, to ustaw, ze spisz, zwolnij kolejke i spij dalej (wyjdz z petli)
        long timeMarker = getMicroTime();
        printf("Time: %ld, Barber: going to sleep...\n", timeMarker);  fflush(stdout);
        sops.sem_num = BARBER;
        sops.sem_op = -1;
        if(semop(SID, &sops, 1) == -1) throw("Barber: 5 sops failed!");

        sops.sem_num = FIFO;
        sops.sem_op = 1;
        if(semop(SID, &sops, 1) == -1) throw("Barber: 6 sops failed!");
        break;
      }
    }
  }
}

pid_t takeChair(struct sembuf* sops){
  sops->sem_num = FIFO;
  sops->sem_op = -1;
  if(semop(SID, sops, 1) == -1) throw("Barber: 1 sops failed!");

  pid_t toCut = fifo->chair;

  sops->sem_op = 1;
  if(semop(SID, sops, 1) == -1) throw("Barber: 2 sops failed!");

  return toCut;
}

void cut(pid_t pid){
  long timeMarker = getMicroTime();
  printf("Time: %ld, Barber: preparing to cut %d\n", timeMarker, pid); fflush(stdout);

  kill(pid, SIGRTMIN);

  timeMarker = getMicroTime();
  printf("Time: %ld, Barber: finished cutting %d\n", timeMarker, pid); fflush(stdout);
}

void prepareFifo(int chNum){
  validateChNum(chNum);

  char* path = getenv(env);
  if(path == NULL) throw("Getting enviromental variable failed!");

  fifoKey = ftok(path, PROJECT_ID);
  if(fifoKey == -1) throw("Barber: getting key of shm failed!");

  shmID = shmget(fifoKey, sizeof(Fifo), IPC_CREAT | IPC_EXCL | 0666);
  if(shmID == -1) throw("Barber: creation of shm failed!");

  void* tmp = (Fifo*) shmat(shmID, NULL, 0);
  if(tmp == (void*)(-1)) throw("Barber: attaching shm failed!");
  fifo = (Fifo*) tmp;

  fifoInit(fifo, chNum);
}

void prepareSemafors(){
  SID = semget(fifoKey, 4, IPC_CREAT | IPC_EXCL | 0666);
  if(SID == -1) throw("Barber: creation of semafors failed!");

  for(int i=1; i<3; i++){
    if(semctl(SID, i, SETVAL, 1) == -1) throw("Barber: Error setting semafors!");
  }
  if(semctl(SID, 0, SETVAL, 0) == -1) throw("Barber: Error setting semafors!");
}

void clearResources(void){
  if(shmdt(fifo) == -1) printf("Barber: Error detaching fifo sm!\n");
  else printf("Barber: detached fifo sm!\n");

  if(shmctl(shmID, IPC_RMID, NULL) == -1) printf("Barber: Error deleting fifo sm!\n");
  else printf("Barber: deleted fifo sm!\n");

  if(semctl(SID, 0, IPC_RMID) == -1) printf("Barber: Error deleting semafors!");
  else printf("Barber: deleted semafors!\n");
}
