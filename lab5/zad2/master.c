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

#include "helpers.h"

int D_X_MIN = -2;
int D_X_MAX = 1;
int D_Y_MIN = -1;
int D_Y_MAX = 1;

void castR(int** tab, int R, double x, double y, int iters){
  int rangeX = D_X_MAX - D_X_MIN;
  int rangeY = D_Y_MAX - D_Y_MIN;

  x -= D_X_MIN;
  y -= D_Y_MIN;

  int rX = (int)(x/rangeX*R);
  int rY = (int)(y/rangeY*R);

  tab[rX][rY] = iters;
}

int main(int argc, char** argv){
  int R = atoi(argv[2]);

  if(mkfifo(argv[1], S_IRUSR| S_IWUSR) == -1){
    printf("Error creating FIFO by master: errno %d\n", errno);
    return 1;
  }

  FILE* fp = fopen(argv[1], "r");
  if(fp == NULL){
    printf("Error opening FIFO by master!\n");
    return 1;
  }

  int** tab = malloc(sizeof(int*) * R);
  for(int i=0; i<R; i++){
    tab[i] = calloc(R, sizeof(int));
  }

  char buffer[100];
  while(fgets(buffer, 100, fp) != NULL){
    double x, y;
    int iters;
    sscanf(buffer, "%lf %lf %d\n", &x, &y, &iters);
    castR(tab, R, x, y, iters);
  }

  // TO DO:
  //zapisz do data
  //uruchom i modl sie
  //fflush(pipe);
  //getc(stdin);
}
