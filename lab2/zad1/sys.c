#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "sys.h"
#include "helpers.h"

int generateSys(char* path, int number, int size){
  int ranDsc = open("/dev/urandom", O_RDONLY);
  int outDsc = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);


  char buff[size];

  for(int i=0; i<number; i++){
    if(read(ranDsc, buff, size) != size){
      printf("There has been an error reading dev/random!\n");
      return 1;
    }
    if(write(outDsc, buff, size) != size){
      printf("There has been an error writing to output file!\n");
      return 1;
    }
  }

  close(ranDsc); close(outDsc);

  return 0;
}

int shuffleSys(char* path, int number, int size){
  int dsc = open(path, O_RDWR);
  char one[size];
  char two[size];
  int ioEr[4];
  int othEr[4];

  for(int i=number - 1; i > 0; i--){
    int r = rand()%(i+1); // 0 <= r <= i
    othEr[0] = lseek(dsc, i*size, SEEK_SET);
    ioEr[0] = read(dsc, one, size);

    othEr[1] = lseek(dsc, r*size, SEEK_SET);
    ioEr[1] = read(dsc, two, size);

    othEr[2] = lseek(dsc, i*size, SEEK_SET);
    ioEr[2] = write(dsc, two, size);

    othEr[3] = lseek(dsc, r*size, SEEK_SET);
    ioEr[3] = write(dsc, one, size);

    if(checkErrors(ioEr, 4, size, othEr, 4, 1, "system shuffling") == 1) return 1;
  }
  close(dsc);
  return 0;
}

int sortSys(char* path, int number, int size){
  int dsc = open(path, O_RDWR);
  char one[size];
  char two[size];

  unsigned char oneFirst[1];
  unsigned char twoFirst[1];

  int ioEr[4];
  int othEr[4];

  for(int i=0; i<number; i++){
    for(int j=0; j<number-i-1; j++){
      othEr[0] = lseek(dsc, j*size, SEEK_SET);
      ioEr[0] = read(dsc, oneFirst, 1);

      othEr[1] = lseek(dsc, (j+1)*size, SEEK_SET);
      ioEr[1] = read(dsc, twoFirst, 1);

      if(checkErrors(ioEr, 2, 1, othEr, 2, 1, "system sorting") == 1) return 1;

      if(*oneFirst > *twoFirst){
        othEr[0] = lseek(dsc, j*size, SEEK_SET);
        ioEr[0] = read(dsc, one, size);

        othEr[1] = lseek(dsc, (j+1)*size, SEEK_SET);
        ioEr[1] = read(dsc, two, size);

        othEr[2] = lseek(dsc, j*size, SEEK_SET);
        ioEr[2] = write(dsc, two, size);

        othEr[3] = lseek(dsc, (j+1)*size, SEEK_SET);
        ioEr[3] = write(dsc, one, size);

        if(checkErrors(ioEr, 4, size, othEr, 4, 1, "system sorting") == 1) return 1;
      }
    }
  }
  close(dsc);
  return 0;
}
