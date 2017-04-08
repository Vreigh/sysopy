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

#include "helpers.h" // popFragment, trimWhite

char** splitToArgv(char* procStr){
  char* procStrCp = concat("", procStr); // tworze tymczasowa kopie procStr, aby ja przerobic i obliczyc liczbe arg
  char* procStrCpOrig = procStrCp; // zapamietuje oryginalny wskaznik na kopie
  int argc = 0;
  for(char* word = popFragment(&procStrCp, ' '); word != NULL; word = popFragment(&procStrCp, ' ')){
    argc++;
  }
  free(procStrCpOrig); // zapamietalem oryginalny wskaznik na kopie, aby zwolnic ta kopie. Concat tworzy w koncu dynamiczny obszar

  char** argv = malloc(sizeof(char*) * (argc + 1));
  argv[argc] = NULL;

  int j = 0;
  for(char* word = popFragment(&procStr, ' '); word != NULL; word = popFragment(&procStr, ' ')){
    argv[j++] = word;
  }

  return argv;
}

int main(){
  printf("Enter your commands separated by '|' symbol\n");
  while(1){
    char* procList = malloc(sizeof(char) * 600);
    fgets(procList, 600, stdin);
    int n = strlen(procList);
    if(n < 2){ // oprocz znaku konca linii musi byc cos jeszcze
      printf("Wrong command line!\n");
      continue;
    }
    if(procList[n - 1] == '\n') procList[--n] = 0; // usun znak konca linii
    procList = realloc(procList, sizeof(char) * n + 1);


    int evenPipe[2];
    int oddPipe[2];
    int k = 0;

    char* orig = procList;
    for(char* i = popFragment(&procList, '|'); i!=NULL; i = popFragment(&procList, '|')){
      char* procStr = trimWhite(i);
      char** argv = splitToArgv(procStr);

      if(k%2 == 0){
        if(k != 0){
          close(evenPipe[0]); close(evenPipe[1]);
        }
        if(pipe(evenPipe) == -1){
          printf("Couldnt pipe at number %d!\n", k);
        }
      }else{
        if(k != 1){
          close(oddPipe[0]); close(oddPipe[1]);
        }
        if(pipe(oddPipe) == -1){
          printf("Couldnt pipe at number %d!\n", k);
        }
      }

      pid_t cp = fork();
      if(cp == -1){
        printf("Couldnt fork child process %d!\n", k);
        return 1;
      }else if(cp == 0){
        //// ustawianie pipow
        if(k%2 == 0){
          if(procList != NULL){
            close(evenPipe[0]);
            if(dup2(1, evenPipe[1]) < 0){
              printf("Couldnt set writing at number %d!\n", k); return 3;
            }
          }
          if(k != 0){
            close(oddPipe[1]);
            if(dup2(0, oddPipe[0]) < 0){
              printf("Couldnt set reading at number %d!\n", k);return 3;
            }
          }
        }else{
          if(procList != NULL){
            close(oddPipe[0]);
            if(dup2(1, oddPipe[1]) < 0){
              printf("Couldnt set writing at number %d!\n", k);return 3;
            }
          }

          close(evenPipe[1]);
          if(dup2(0, evenPipe[0])< 0){
            printf("Couldnt set reading at number %d!\n", k); return 3;
          }
        }
        ////
        execvp(argv[0], argv);
        printf("ERROR EXECUTING CHILD PROCESS %d!\n", k);
        return 2;
      }
      k++;
      free(procStr); free(argv);
    }
    free(orig);

    while(1){
      wait(NULL);
      if (errno == ECHILD){
          printf("\nWhole line has been executed!\n");
          break;
      }
    }
    usleep(10000); // zeby w razie errorow napis "next commands" i tak pojawial sie na koncu
    printf("\nNext commands: ");
  }
  return 0;
}
