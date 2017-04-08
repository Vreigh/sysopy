#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <complex.h>
#include <time.h>
#include <math.h>

#include "helpers.h"

int D_X_MIN = -2;
int D_X_MAX = 1;
int D_Y_MIN = -1;
int D_Y_MAX = 1;

double complex generatePoint(){
  double x = (double)rand()/RAND_MAX *(D_X_MAX - D_X_MIN) + D_X_MIN;
  double y = (double)rand()/RAND_MAX *(D_Y_MAX - D_Y_MIN) + D_Y_MIN;
  return x + y *I;
}

void generate(int K, int fd){
  double complex c = generatePoint();
  double complex tmp = c;
  int k = 0;
  while(cabs(tmp) <= 2){
    k++;
    tmp = tmp*tmp + c;
    if(k == K) break;
  }
  char buffer[100];
  sprintf(buffer, "%.3f %.3f %d\n", creal(c), cimag(c), k);
  write(fd, buffer, strlen(buffer));
}

int main(int argc, char** argv){
  int N = atoi(argv[2]);
  int K = atoi(argv[3]);

  int fd = open(argv[1], O_WRONLY); // bede wiedzial ile bajtow chce zapisac - wystarczy mi open
  if(fd < 0){
    printf("Error opening FIFO - one of the slaves!\n");
  }
  srand(time(NULL));
  for(int i=0; i<N; i++){
    generate(K, fd);
  }
  return 0;
}
