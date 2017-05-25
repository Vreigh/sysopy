// głodzenie pisarzy, semafory
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <semaphore.h>

#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

#include "helpers.h"

void showUsage();
int validateTN(int n);
int validateWN(int n);
int validateRN(int n);
pthread_t* prepareWriters(int n);
pthread_t* prepareReaders(int n);
void* wJob(void*);
void* rJob(void*);
void change(int *tab, int n, int list);
void search(int* tab, int n, long mod, int list);

const int maxWN = 100;
const int maxRN = 100;

sem_t* checker;
sem_t* writer;
sem_t* wWaiter;

int* tab;
Users users;
int list = 0;

int main(int argc, char** argv){
  if((argc != 3) && (argc != 4)) showUsage();
  int rN, wN;
  if(argc == 3){
    wN = validateWN(atoi(argv[1]));
    rN = validateRN(atoi(argv[2]));
  }else{
    if( strcmp(argv[1], "-i") != 0) showUsage();
    list = 1;
    wN = validateWN(atoi(argv[2]));
    rN = validateRN(atoi(argv[3]));
  }
  srand(time(NULL)*getpid()>>1);

  checker = malloc(sizeof(sem_t));
  writer = malloc(sizeof(sem_t));
  wWaiter = malloc(sizeof(sem_t));

  if(sem_init(checker, 0, 1) == -1) throw("Creating checker semaphore failed!");
  if(sem_init(writer, 0, 1) == -1) throw("Creating writer semaphore failed!");
  if(sem_init(wWaiter, 0, 0) == -1) throw("Creating waiter semaphore failed!");

  tab = getRandomTable(tabSize);
  users.r = 0;
  users.w = -1;

  pthread_t* writers = prepareWriters(wN);
  pthread_t* readers = prepareReaders(rN);

  for(int i=0; i<wN; i++){
    if(pthread_join(writers[i], NULL) != 0) throw("joining writer failed!");
  }
  for(int i=0; i<rN; i++){
    if(pthread_join(readers[i], NULL) != 0) throw("joining reader failed!");
  }

  return 0;
}

void* wJob(void* arg){
  printf("writer %ld is here, hello!\n", gettid());

  while(1){
    if(sem_wait(writer) == -1) throw("writer wait failed!"); // jeden pisarz naraz - duze faworyzowanie czytelnikow
    if(sem_wait(checker) == -1) throw("checker wait failed!");

    users.w = gettid(); // ustaw siebie na oczekujacego

    if(users.r > 0){ // jesli jeszcze sa jacys czytelnicy
      if(sem_post(checker) == -1) throw("wJob inner checker post failed!"); // pozwalam dzialac czytelnikom

      if(sem_wait(wWaiter) == -1) throw("wJob wWaiter wait failed!"); // czekam na "sygnal" od ostatniego czytelnika
      change(tab, tabSize, list); // robie swoje
      users.w = -1; // robie swoje - ustawiam, ze mnie tu juz nie ma

      if(sem_post(checker) == -1) throw("wJob checker inner post failed!");
      if(sem_post(writer)==-1) throw("wJob writer inner post failed!"); // zamierzona zamiana kolejnosci, zeby kolejny writer zdazyl zaczac czekac

      sleep(1);
    }else{
      change(tab, tabSize, list);
      users.w = -1;

      if(sem_post(checker) == -1) throw("wJob post checker outer failed!");
      if(sem_post(writer) == -1) throw("wJob post writer outer failed!");

      //sleep(1);
    }
  }

  return NULL;
}

void* rJob(void* arg){
  long mod = (long)arg;
  printf("reader %ld is here, i will be searching for %ld, hello!\n", gettid(), mod);

  while(1){
    if(sem_wait(checker) == -1) throw("a ");
    users.r++;
    if(sem_post(checker) == -1) throw("b");

    search(tab, tabSize, mod, list);

    if(sem_wait(checker) == -1) throw("c");
    users.r--;
    if((users.r == 0) && (users.w != -1)){
      if(sem_post(wWaiter) == -1) throw("d"); // obudz pisarza, "przekaz" mu semafor, nie zwalniajac go
    }else{
      if(sem_post(checker) == -1) throw("e"); // jesli warunki nie sa spelnione, po prostu zwolnij checker
    }

    //sleep(1);
  }

  return NULL;
}

void change(int *tab, int n, int list){
  int toChange = rand()%(tabSize/50); // zeby nie zapchac stdout
  if(toChange < 1) toChange = 1;

  for(int i=0; i<toChange; i++){
    int j = rand()%tabSize;
    int number = rand() * rand();

    tab[j] = number;
    if(list == 1) printf("CHANGING - Index: %d, Number: %d\n", j, number);
  }
  printf("I, %ld, have changed %d numbers!\n", gettid(), toChange);
}

void search(int* tab, int n, long mod, int list){
  int found = 0;
  for(int i=0; i<n; i++){
    if(tab[i]%mod == 0){
      found++;
      if(list == 1) printf("FOUND: Index: %d, Number: %d, Mod: %ld\n", i, tab[i], mod);
    }
  }
  printf("I, %ld, have found %d numbers divided by %ld\n", gettid(), found, mod);
}

pthread_t* prepareWriters(int n){
  pthread_t* writers = malloc(sizeof(pthread_t)*n);
  for(int i=0; i<n; i++){
    if(pthread_create(&writers[i], NULL, wJob, NULL) != 0) throw("Creating thread failed!");
  }
  return writers;
}
pthread_t* prepareReaders(int n){
  pthread_t* readers = malloc(sizeof(pthread_t)*n);
  for(int i=0; i<n; i++){
    long arg = (long)(rand()%100) + 1;
    if(pthread_create(&readers[i], NULL, rJob, (void*)arg) != 0) throw("Creating thread failed!");
  }
  return readers;
}

void showUsage(){
  printf("Use program like ./rwr.out <wN> <rN>\n");
  exit(1);
}
int validateWN(int n){
  if(n < 1 || n > maxWN){
    printf("wN must be between 1 and %d\n", maxWN);
    exit(1);
  }
  return n;
}
int validateRN(int n){
  if(n < 1 || n > maxRN){
    printf("rN must be between 1 and %d\n", maxRN);
    exit(1);
  }
  return n;
}
