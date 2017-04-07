#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#include "helpers.h" // popFragment, trimWhite

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
    if(procList[n - 1] == '\n') procList[n - 1] = 0; // usun znak konca linii

    char* procStr = popFragment(&procList, '|');
    int i = 1;
    while(procStr != NULL){
      //procStr = trimWhite(procStr);
      //createProc(procStr);
      //free(procStr);
      printf("\n%d: %s", i, procStr);
      procStr = popFragment(&procList, '|');
      i++;
    }
    printf("\nNext line: ");
  }
  return 0;
}
