#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lib.h"
#include "helpers.h"


int generateLib(char* path, int number, int size){
  FILE* outDsc = fopen(path, "w");
  FILE* ranDsc = fopen("/dev/urandom", "r");

  char buff[size];

  for(int i=0; i<number; i++){
    if(fread(buff, 1, size, ranDsc) != size){
      printf("There has been an error reading dev/random!\n");
      return 1;
    }
    if(fwrite(buff, 1, size, outDsc) != size){
      printf("There has been an error writing to output file!\n");
      return 1;
    }
  }
  fclose(outDsc); fclose(ranDsc);

  return 0;
}

int shuffleLib(char* path, int number, int size){
  FILE* dsc = fopen(path, "r+");
  char one[size];
  char two[size];
  int ioEr[4];
  int othEr[4];

  for(int i=number - 1; i > 0; i--){
    int r = rand()%(i+1); // 0 <= r <= i
    othEr[0] = fseek(dsc, i*size, SEEK_SET);
    ioEr[0] = fread(one, 1, size, dsc);

    othEr[1] = fseek(dsc, r*size, SEEK_SET);
    ioEr[1] = fread(two, 1, size, dsc);

    othEr[2] = fseek(dsc, i*size, SEEK_SET);
    ioEr[2] = fwrite(two, 1, size, dsc);

    othEr[3] = fseek(dsc, r*size, SEEK_SET);
    ioEr[3] = fwrite(one, 1, size, dsc);

    if(checkErrors(ioEr, 4, size, othEr, 4, 0, "lib shuffling") == 1) return 1;
  }
  fclose(dsc);
  return 0;
}

int sortLib(char* path, int number, int size){
  FILE* dsc = fopen(path, "r+");
  char one[size];
  char two[size];

  unsigned char oneFirst[1];
  unsigned char twoFirst[1];

  int ioEr[4];
  int othEr[4];

  for(int i=0; i<number; i++){
    for(int j=0; j<number-i-1; j++){
      othEr[0] = fseek(dsc, j*size, SEEK_SET);
      ioEr[0] = fread(oneFirst, 1, 1, dsc);

      othEr[1] = fseek(dsc, (j+1)*size, SEEK_SET);
      ioEr[1] = fread(twoFirst, 1, 1, dsc);

      if(checkErrors(ioEr, 2, 1, othEr, 2, 0, "lib sorting") == 1) return 1;

      if(*oneFirst > *twoFirst){
        othEr[0] = fseek(dsc, j*size, SEEK_SET);
        ioEr[0] = fread(one, 1, size, dsc);

        othEr[1] = fseek(dsc, (j+1)*size, SEEK_SET);
        ioEr[1] = fread(two, 1, size, dsc);

        othEr[2] = fseek(dsc, j*size, SEEK_SET);
        ioEr[2] = fwrite(two, 1, size, dsc);

        othEr[3] = fseek(dsc, (j+1)*size, SEEK_SET);
        ioEr[3] = fwrite(one, 1, size, dsc);

        if(checkErrors(ioEr, 4, size, othEr, 4, 0, "lib sorting") == 1) return 1;
      }
    }
  }
  fclose(dsc);
  return 0;
}
