// głodzenie czytelników, mutexy
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

pthread_mutex_t checker = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t wFinish =  PTHREAD_COND_INITIALIZER;
pthread_cond_t rFinish =  PTHREAD_COND_INITIALIZER;

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
    pthread_mutex_lock(&checker);

    while(users.w != -1){
      pthread_cond_wait(&wFinish, &checker);
    }
    users.w = gettid();

    while(users.r > 0){ // poczekaj az wszyscy czytelnicy skoncza
      pthread_cond_wait(&rFinish, &checker);
    }

    change(tab, tabSize, list);
    users.w = -1;

    pthread_cond_broadcast(&wFinish);
    pthread_mutex_unlock(&checker);

    sleep(1);
  }

  return NULL;
}

void* rJob(void* arg){
  long mod = (long)arg;
  printf("reader %ld is here, i will be searching for %ld, hello!\n", gettid(), mod);

  while(1){
    pthread_mutex_lock(&checker);
    while(users.w != -1){
      pthread_cond_wait(&wFinish, &checker);
    }
    users.r++;
    pthread_mutex_unlock(&checker);

    search(tab, tabSize, mod, list);

    pthread_mutex_lock(&checker);
    users.r--;
    if(users.r == 0) pthread_cond_signal(&rFinish);
    pthread_mutex_unlock(&checker);

    sleep(1);
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
