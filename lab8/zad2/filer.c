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

#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

#include "helpers.h"

void showUsage(){
  printf("Use program like ./filer.out <filename> <R>\n");
  exit(1);
}
int validateR(int num){
  if(num < 2 || num > 5000){
    throw("Wrong number of Records!");
    return -1;
  }
  else return num;
}

int main(int argc, char** argv){
  srand(time(NULL));

  if(argc != 3) showUsage();

  char* fname = argv[1];
  int R = validateR(atoi(argv[2]));

  int fd = open(fname, O_WRONLY | O_TRUNC | O_CREAT, 0666);
  if(fd == -1) throw("Opening file failed!");

  for(int i=0; i<R; i++){
    if(write(fd, &i, sizeof(int)) == -1) throw("Writing failed!");
    for(int j=0; j<1020; j++){
      char ran = 'a' + (char)(rand()%26);
      if(write(fd, &ran, 1) == -1) throw("Writing failed!");
    }
  }

  printf("%ld %ld\n", sizeof(int), sizeof(void*));

  return 0;
}
