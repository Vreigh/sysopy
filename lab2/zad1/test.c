#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


int main(int argc, char** argv){
  int one = open(argv[1], O_RDONLY);
  int two = open(argv[2], O_RDONLY);
  int number = atoi(argv[3]);
  int size = atoi(argv[4]);

  char buff1[size];
  char buff2[size];

  for(int i=0; i<number; i++){
    read(one, buff1, size);
    read(two, buff2, size);
    for(int j=0; j<size; j++){
      if(buff1[j] != buff2[j]){
        printf("cos jest nie tak\n");
        return 1;
      }
    }
  }
  printf("sa takie same\n");
  return 0;
}
