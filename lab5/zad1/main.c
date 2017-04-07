#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#include "helpers.h" // popFragment, trimWhite

void createProc(char* procStr){
  char* orig = procStr;

  char* procStrCp = concat("", procStr);
  char* procStrCpOrig = procStrCp;
  int argc = 0;
  char* word = popFragment(&procStrCp, ' ');
  while(word != NULL){
    argc++;
    word = popFragment(&procStrCp, ' ');
  }
  free(procStrCpOrig);

  char* argv[argc + 1];
  argv[argc] = NULL;
  int j = 0;

  word = popFragment(&procStr, ' ');
  while(word != NULL){
    argv[j++] = word;
    word = popFragment(&procStr, ' ');
  }

  for(int i=0; i<argc; i++){
    printf("%s\n", argv[i]);
  }
  printf("------------------------------------\n");

  free(orig);
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
    char* orig = procList;

    char* procStr = popFragment(&procList, '|');
    int i = 1;
    while(procStr != NULL){
      procStr = trimWhite(procStr);
      createProc(procStr);
      procStr = popFragment(&procList, '|');
      i++;
    }
    free(orig);
    printf("\nNext commands: ");
  }
  return 0;
}
