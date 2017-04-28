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
  if(num < 0 || num > 1000) throw("Wrong number of Chairs!");
}
void intHandler(int signo){
  exit(2);
}

void clearResources(void);
void prepareFifo(int chNum);
void prepareSemafors();
void napAndWorkForever();

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
    sops.sem_num = 0;
    sops.sem_op = -1;
    sops.sem_flg = 0;

    if(semop(SID, &sops, 1) == -1) throw("Barber: nap sops failed!");
    // zwiekszam semafor do 1
    // odblokowuje spiacego klienta, ktory mnie obudzil - ten klient bedzie juz mogl zwolnic kolejke
    // strzyze krzeslo
    // wyjmuje kolejnych klientow z kolejki na krzeslo i strzyze krzeslo
    // w ostatniej iteracji zmniejszam sem do 0 (UWAGA: PRZED ZWOLNIENIEM KOLEJKI)
    // zwalniam kolejke i znowu spac
    // nie ma znaczenia, czy zdaze sie zablokowac, czy jeszcze przed pierwszym semop jakis klient
    // mnie obudzi - efekt bedzie ten sam: obudze sie i zaczne strzyc
  }
}

void prepareFifo(int chNum){
  validateChNum(chNum);

  char* path = getenv(env);
  if(path == NULL) throw("Getting enviromental variable failed!");

  fifoKey = ftok(path, PROJECT_ID);
  if(fifoKey == -1) throw("Barber: getting key of sm failed!");

  shmID = shmget(fifoKey, sizeof(Fifo), IPC_CREAT | IPC_EXCL | 0666);
  if(shmID == -1) throw("Barber: creation of sm failed!");

  void* tmp = (Fifo*) shmat(shmID, NULL, 0);
  if(tmp == (void*)(-1)) throw("Barber: attaching sm failed!");
  fifo = (Fifo*) tmp;

  fifoInit(fifo, chNum);
}

void prepareSemafors(){
  SID = semget(fifoKey, 3, IPC_CREAT | IPC_EXCL | 0666);
  if(SID == -1) throw("Barber: creation of semafors failed!");

  for(int i=0; i<3; i++){
    if(semctl(SID, i, SETVAL, 0) == -1) throw("Barber: Error setting semafors!");
  }
}

void clearResources(void){
  if(shmdt(fifo) == -1) printf("Barber: Error detaching fifo sm!\n");
  else printf("Barber: detached fifo sm!\n");

  if(shmctl(shmID, IPC_RMID, NULL) == -1) printf("Barber: Error deleting fifo sm!\n");
  else printf("Barber: deleted fifo sm!\n");

  if(semctl(SID, 0, IPC_RMID) == -1) printf("Barber: Error deleting semafors!");
  else printf("Barber: deleted semafors!\n");
}
