#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "helpers.h"

int checkErrors(int* ioEr, int ioN, int size, int* othEr, int othN, int isSystem, char* desc){
  for(int i=0; i<ioN; i++){
    if(ioEr[i] != size){
      printf("There has been an error while %s with i/o operations!\n Number: %d \n", desc, i);
      return 1;
    }
  }

  for(int i=0; i<othN; i++){
    if(isSystem == 1){
      if(othEr[i] == -1){
        printf("There has been an error while %s with other operations!\n Number: %d \n", desc, i);
        return 1;
      }
    }else if(isSystem == 0){
      if(othEr[i] != 0){
        printf("There has been an error while %s with other operations!\n Number: %d \n", desc, i);
        return 1;
      }
    }
  }

  return 0;
}

int cloneFile(char* orig, char* dest, int number, int size){
  int origDsc = open(orig, O_RDONLY);
  int destDsc = open(dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  char buff[size];

  for(int i=0; i<number; i++){
    if(read(origDsc, buff, size) != size){
      printf("There has been an error copying file %d X %d! \n", number, size);
      return 1;
    }
    if(write(destDsc, buff, size) != size){
      printf("There has been an error copying file %d X %d! \n", number, size);
      return 1;
    }
  }
  close(origDsc); close(destDsc);
  return 0;
}
