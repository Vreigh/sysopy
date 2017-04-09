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
volatile int allSlavesRdy = 0;

void rtHandler(int signo){
  if(signo == SIGRTMIN) allSlavesRdy++;
}

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
  if(signal(SIGRTMIN, rtHandler) == SIG_ERR){
    printf("Error setting handler by master %d\n", errno);
    kill(getppid(), SIGRTMIN+2);
    return 1;
  }
  int R = atoi(argv[2]);

  if(mkfifo(argv[1], S_IRUSR| S_IWUSR) == -1){
    printf("Error creating FIFO by master: errno %d\n", errno);
    kill(getppid(), SIGRTMIN+2);
    return 1;
  }

  printf("Fifo has been created!\n");
  kill(getppid(), SIGRTMIN+1);

  FILE* fp = fopen(argv[1], "r");
  if(fp == NULL){
    printf("Error opening FIFO by master!\n");
    kill(getppid(), SIGRTMIN+2);
    return 1;
  }

  while(allSlavesRdy != 1){
    sleep(1);
  }
  printf("I, master, received a signal, that all slaves are already writing!\n");

  int** tab = malloc(sizeof(int*) * R);
  for(int i=0; i<R; i++){
    tab[i] = calloc(R, sizeof(int));
  }

  char buffer[100];
  while(fgets(buffer, 100, fp) != NULL){
    double x, y;
    int iters;
    sscanf(buffer, "%lf %lf %d\n", &x, &y, &iters);
    castR(tab, R-1, x, y, iters);
  }
  printf("I, master, left reading fifo!\n");

  FILE* data = fopen("data.txt", "w");
  for(int i=0; i<R; i++){
    for(int j=0; j<R; j++){
      fprintf(data, "%d %d %d\n", i, j, tab[i][j]);
    }
  }
  fclose(data);

  if(remove("fifo") == -1){
    printf("Error deleting fifo!");
    return 3;
  }
  printf("Successfully terminated - fifo removed!\n");

  FILE* gnup = popen("gnuplot", "w");
  fprintf(gnup, "set view map\n");
  fprintf(gnup, "set xrange [0:%d]\n", R);
  fprintf(gnup, "set yrange [0:%d]\n", R);
  fprintf(gnup, "plot 'data.txt' with image\n");
  fflush(gnup);

  getc(stdin);
  pclose(gnup);

  return 0;
}
