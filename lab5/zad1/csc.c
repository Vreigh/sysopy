#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char** argv){
  char buffer[1000];

  if(argc < 2) return 1;
  if(strcmp(argv[1], "w") == 0){
    if(argc < 3) return 1;
    int tmp = open("tmp.txt", O_RDWR|O_TRUNC); // chce sprawdzic, czy wszystko zadzialało jak myśle
    write(tmp, argv[2], strlen(argv[2]));
    write(1, argv[2], strlen(argv[2]));
    close(tmp);
  }else if(strcmp(argv[1], "rw") == 0){
    int tmp = open("tmp.txt", O_RDWR|O_APPEND); // chce sprawdzic, czy wszystko zadzialało jak myśle
    int n = read(0, buffer, 1000);
    if(n == 0){
      write(tmp, "nope\n", 5);
    }
    write(tmp, buffer, n);
    write(1, buffer, n);
    write(tmp, "done\n", 5);
    close(tmp);
  }else return 2;

  return 0;
}
