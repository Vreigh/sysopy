// przerwania asynchroniczne
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
int validateTNum(int tNum);
int validateR(int R);
char* validateFilename(char* name);
char* validateWord(char* word);

void* threadJob(void*);
void decLeft();

FILE* fp;
int toRead;
char* word;
int tNum;
pthread_t* threads;
pthread_mutex_t fileAccess = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t checker = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t checkerSig =  PTHREAD_COND_INITIALIZER;
int left;

int main(int argc, char** argv){
  if(argc != 5) showUsage();

  tNum = validateTNum(atoi(argv[1]));
  char* fname = validateFilename(argv[2]);
  toRead = validateR(atoi(argv[3]));
  word = concat("", validateWord(argv[4])); // kopia slowa na stercie

  left = tNum;

  fp = fopen(fname, "r");
  if(fp == NULL) throw("Opening file failed!");

  threads = malloc(tNum * sizeof(pthread_t));

  pthread_attr_t thAttr;
  if(pthread_attr_init(&thAttr) != 0) throw("pthread_attr_init failed!");
  if(pthread_attr_setdetachstate(&thAttr, PTHREAD_CREATE_DETACHED) != 0) throw("setting attr failed!");

  for(long i=0; i<tNum; i++){
    if(pthread_create(&threads[i], &thAttr, threadJob, (void*)i) != 0) throw("Creating thread failed!");
  }

  pthread_mutex_lock(&checker);
  while(left != 0) pthread_cond_wait(&checkerSig, &checker);
  pthread_mutex_unlock(&checker);

  printf("All done\n");
  return 0;
}

void* threadJob(void* arg){
  Record* records = malloc(sizeof(Record) * toRead);

  // rozpocznij czytanie
  int loopEnd = 0;
  while(1){
    pthread_mutex_lock(&fileAccess);
    for(int i=0; i<toRead; i++){
      if(fread(&records[i].id, sizeof(int), 1, fp) != 1){
        loopEnd = 1;
        break;
      }

      if(fread(records[i].text, 1, 1020, fp) != 1020) throw("reading text failed!");
      records[i].text[1020] = '\0';
    }
    pthread_mutex_unlock(&fileAccess);
    if(loopEnd == 1) break;

    for(int i=0; i<toRead; i++){
      if(strstr(records[i].text, word) != NULL){
        printf("I, thread %ld, of index %ld, have succeded! The word %s has been found in record: %d!\n",
        gettid(),(long)arg, word, records[i].id);

        free(records);
        decLeft();
        return NULL;
      }
    }
  }
  // doszedlem do konca pliku bez skutku - nic nie rob, po prostu sie skoncz
  printf("No success!\n");
  free(records);
  decLeft();
  return NULL;
}

void decLeft(){
  pthread_mutex_lock(&checker);
  left--;
  pthread_cond_signal(&checkerSig);
  pthread_mutex_unlock(&checker);
}

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
