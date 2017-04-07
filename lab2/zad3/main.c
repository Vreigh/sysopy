#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "helpers.h"

int putLock(int cmd, int l_type, int byte, int dsc);
void listLocks(int holder);
void tryRead(int dsc, int byte);
void tryWrite(int dsc, int byte);

void printUsage(){
  printf("Use file like ./exec.out <filename> \n");
}

void showResult(int r){
  if(r == 0) printf("Couldnt put a lock!\n");
  else if(r == 1) printf("Lock has been put!\n");
}

void printMenu(){
  printf("type: \n");
  printf("lkr to set a reading lock\n");
  printf("lkrw to set a reading lock with waiting\n");
  printf("lkw to set a writing lock\n");
  printf("lkww to set a writing lock with waiting\n");
  printf("lks to list all locks\n");
  printf("lkun to remove a lock\n");
  printf("r to read a char\n");
  printf("w to write a char\n");
  printf("menu to show the menu again\n");
  printf("q to quit\n");
}

int main(int argc, char** argv){
  char* byteQuestion = "Enter the number of a byte: ";

  if(argc != 2){ printUsage(); return 1;}

  int dsc;
  if( (dsc = open(argv[1], O_RDWR)) < 0){
    printf("Error opening Your file! \n");
    return 1;
  }

  char* input = calloc(10, sizeof(char));
  printMenu();
  while(strcmp(input, "q") != 0){
    printf("\nPlease write your command: ");
    int byte;
    scanf("%s", input);

    if(strcmp(input, "lkr") == 0){
      printf("%s", byteQuestion);
      scanf("%d", &byte);
      showResult(putLock(F_SETLK, F_RDLCK, byte, dsc));
    }else if(strcmp(input, "lkrw") == 0){
      printf("%s", byteQuestion);
      scanf("%d", &byte);
      showResult(putLock(F_SETLKW, F_RDLCK, byte, dsc));
    }else if(strcmp(input, "lkw") == 0){
      printf("%s", byteQuestion);
      scanf("%d", &byte);
      showResult(putLock(F_SETLK, F_WRLCK, byte, dsc));
    }else if(strcmp(input, "lkww") == 0){
      printf("%s", byteQuestion);
      scanf("%d", &byte);
      showResult(putLock(F_SETLKW, F_WRLCK, byte, dsc));
    }else if(strcmp(input, "lks") == 0){
      listLocks(dsc);
    }else if(strcmp(input, "lkun") == 0){
      printf("%s", byteQuestion);
      scanf("%d", &byte);
      showResult(putLock(F_SETLK, F_UNLCK, byte, dsc));
    }else if(strcmp(input, "r") == 0){
      printf("%s", byteQuestion);
      scanf("%d", &byte);
      tryRead(dsc, byte);
    }else if(strcmp(input, "w") == 0){
      printf("%s", byteQuestion);
      scanf("%d", &byte);
      tryWrite(dsc, byte);
    }
    else if(strcmp(input, "menu") == 0){
      printMenu();
    }
  }
  free(input);
  close(dsc);
}

void tryWrite(int dsc, int byte){
  if(putLock(F_SETLK, F_WRLCK, byte, dsc) == 0){
    printf("Byte is blocked, cant be written to!\n");
  }else{
    if(lseek(dsc, byte, SEEK_SET) == -1){
      printf("Error setting descriptor!\n");
    }else{
      char* out = calloc(1, sizeof(char));
      printf("Enter character to write: ");
      scanf(" %c", out); // skip leading whitespace (%c doesnt skip automaticly)

      if(write(dsc, out, 1) != 1){
        printf("Error reading Your file!\n");
      }else{
        printf("Successfully written char: %c \n", *out);
      }
    }
    putLock(F_SETLK, F_UNLCK, byte, dsc);
  }
}

void tryRead(int dsc, int byte){
  if(putLock(F_SETLK, F_RDLCK, byte, dsc) == 0){
    printf("Byte is blocked, cant be read!\n");
  }else{
    if(lseek(dsc, byte, SEEK_SET) == -1){
      printf("Error setting descriptor!\n");
    }else{
      char* out = calloc(1, sizeof(char));
      if(read(dsc, out, 1) != 1){
        printf("Error reading Your file!\n");
      }else{
        printf("Read char: %c \n", *out);
      }
    }
    putLock(F_SETLK, F_UNLCK, byte, dsc);
  }
}

int putLock(int cmd, int l_type, int byte, int dsc){
  struct flock* fl = malloc(sizeof(struct flock));
  fl->l_type = l_type;
  fl->l_whence = SEEK_SET;
  fl->l_start = byte;
  fl->l_len = 1;
  if(fcntl(dsc, cmd, fl) == -1) return 0;
  else return 1;
  free(fl);
}

void listLocks(int dsc) {
  printf("Listing..\n");
  struct flock *fl = malloc(sizeof(struct flock));
  fl->l_whence = SEEK_SET;
  fl->l_len = 1;
  fl->l_type = F_WRLCK;

  int offset;
  if ((offset = lseek(dsc, 0, SEEK_END)) == -1) {
    perror("Error performing lseek!\n");
    return;
  }
  for (int byte = 0; byte <= offset; byte++) {
    fl->l_start = byte;
    fl->l_type = F_WRLCK; //F_WRLCK, becouse read can be put on other reads

    fcntl(dsc, F_GETLK, fl);

    if(fl->l_type == F_UNLCK) continue;
    if (fl -> l_type == F_RDLCK) printf("Byte: %d, READING lock put by PID %d\n", byte, fl->l_pid);
    else if (fl -> l_type == F_WRLCK) printf("Byte: %d, WRITING lock put by PID %d\n", byte, fl->l_pid);
  }
  free(fl);
}
