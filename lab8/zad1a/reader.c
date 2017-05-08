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

void showUsage(){
  printf("Use program like ./reader.out <threads> <filename> <toRead> <word>\n");
  exit(1);
}

int validateTNum(int tNum){
  if((tNum < 1) || (tNum > 50)){
    throw("Wrong number of threads!");
    return -1;
  }
  return tNum;
}
int validateR(int R){
  if((R < 1) || (R > 10)){
    throw("Wrong R!");
    return -1;
  }
  return R;
}
char* validateFilename(char* name){
  if(access(name, F_OK) == -1){
    throw("Wrong filename!");
    return NULL;
  }
  return name;
}
char* validateWord(char* word){
  if(strlen(word) > 10 ){
    throw("Wrong word!");
    return NULL;
  }
  return word;
}

FILE* fp;
int toRead;
char* word;
pthread_t* threads;
pthread_mutex_t fileAccess;

void* threadJob(void*);

int main(int argc, char** argv){
  if(argc != 5) showUsage();

  int tNum = validateTNum(atoi(argv[1]));
  char* fname = validateFilename(argv[2]);
  toRead = validateR(atoi(argv[3]));
  word = argv[4];

  fp = fopen(fname, "r");
  if(fp == NULL) throw("Opening file failed!");

  threads = malloc(tNum * sizeof(pthread_t));

  pthread_attr_t thAttr;
  if(pthread_attr_init(&thAttr) != 0) throw("pthread_attr_init failed!");

  for(int i=0; i<tNum; i++){
    threads[i] = i;
    if(pthread_create(&threads[i], &thAttr, threadJob, NULL) != 0) throw("Creating thread failed!");
  }

  for(int i=0; i<tNum; i++){
    if(pthread_join(threads[i], NULL) != 0) throw("joining failed!");
  }

  return 0;
}

void* threadJob(void* arg){
  printf("Hello! Im a new thread. My TID is: %ld\n", gettid());
  // ustaw sposob cancelowania
  // ustaw funkcje czyszczaca
  // rozpocznij czytanie
  return NULL;
}
