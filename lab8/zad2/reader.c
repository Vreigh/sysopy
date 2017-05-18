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
void handleSuccess();

void handler(int signo){
  char buffer[100];
  sprintf(buffer, "\nReceived %d, PID: %d, TID: %ld\n", signo, getpid(), gettid());
  write(1, buffer, strlen(buffer)+1);
}

FILE* fp;
int toRead;
char* word;
int tNum;
pthread_t* threads;
pthread_mutex_t fileAccess = PTHREAD_MUTEX_INITIALIZER;
pthread_key_t* keys;


int main(int argc, char** argv){
  if(argc != 5) showUsage();

  tNum = validateTNum(atoi(argv[1]));
  char* fname = validateFilename(argv[2]);
  toRead = validateR(atoi(argv[3]));
  word = concat("", validateWord(argv[4])); // kopia slowa na stercie

//////////////////////////////////////////
  /*sigset_t block;
  sigemptyset(&block);
  sigaddset(&block, SIGUSR1);
  sigaddset(&block, SIGTERM);

  if(pthread_sigmask(SIG_BLOCK, &block, NULL) == -1) throw("Setting mask failed!");*/

  if(signal(SIGUSR1, handler) == SIG_ERR) throw("Setting handler failed!");
  if(signal(SIGTERM, handler) == SIG_ERR) throw("Setting handler failed!");

///////////////////////////////////////////

  fp = fopen(fname, "r");
  if(fp == NULL) throw("Opening file failed!");

  printf("My PID is: %d", getpid()); fflush(stdout);

  threads = malloc(tNum * sizeof(pthread_t));
  keys = malloc(tNum * sizeof(pthread_key_t));

  pthread_attr_t thAttr;
  if(pthread_attr_init(&thAttr) != 0) throw("pthread_attr_init failed!");

  for(long i=0; i<tNum; i++){
    if(pthread_key_create(&keys[i], NULL) != 0) throw("Creating key failed!");
    if(pthread_create(&threads[i], &thAttr, threadJob, (void*)i) != 0) throw("Creating thread failed!");
  }

  sleep(1);
  for(int i=0; i<tNum; i++){
    if(pthread_kill(threads[i], SIGTERM) != 0) throw("Killing failed!");
    if(pthread_kill(threads[i], SIGUSR1) != 0) throw("Killing failed!");
  }

  for(int i=0; i<tNum; i++){
    if(pthread_join(threads[i], NULL) != 0) throw("joining failed!");
    Record* toDelete = (Record*)pthread_getspecific(keys[i]);
    if(toDelete != NULL) free(toDelete);
  }

  printf("All done\n");

  return 0;
}

void* threadJob(void* arg){
  if(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL) != 0) throw("Error setting cancel state!");

  if((long)arg == 3){
    //int crash = 1 / 0;
  }

  /*sigset_t block;
  sigemptyset(&block);
  sigaddset(&block, SIGUSR1);
  sigaddset(&block, SIGTERM);
  if(pthread_sigmask(SIG_BLOCK, &block, NULL) == -1) throw("Setting mask failed!");*/

  Record* records = malloc(sizeof(Record) * toRead);
  if(pthread_setspecific(keys[(long)arg], records) != 0) throw("setting key failed!");

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

    slower(11500);

    for(int i=0; i<toRead; i++){
      if(strstr(records[i].text, word) != NULL){
        printf("I, thread %ld, of index %ld, have succeded! The word %s has been found in record: %d!\n",
        gettid(),(long)arg, word, records[i].id);

        handleSuccess();
        printf("I handled my success!\n");
        return NULL;
      }
    }

    if(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) != 0) throw("Error setting cancel state!");
    pthread_testcancel();
    if(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL) != 0) throw("Error setting cancel state!");
  }
  // doszedlem do konca pliku bez skutku - nic nie rob, po prostu sie skoncz
  printf("No success!\n");
  return NULL;
}

void handleSuccess(){
  pthread_t self = pthread_self();
  for(int i=0; i<tNum; i++){
    if(pthread_equal(self, threads[i]) == 0){
      printf("I, thread %ld, will try to cancel %i fella!\n", gettid(), i);
      if(pthread_cancel(threads[i]) != 0){
        printf("I, thread %ld failed to cancel thread of index %d!\n", gettid(), i);
      }
    }
  }
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
  if(strlen(word) > 25 ){
    throw("Wrong word!");
    return NULL;
  }
  return word;
}
