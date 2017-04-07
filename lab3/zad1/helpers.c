#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helpers.h"

char* concat(const char* one, const char* two){
    char* new = malloc(sizeof(char) * (strlen(one) + strlen(two) + 1) );
    if(strcmp(one, "") != 0) strcpy(new, one);
    strcat(new, two);
    return new;
}
char* concat3(char *s1, char *s2, char* s3){
  char* new = malloc(sizeof(char) * (strlen(s1) + strlen(s2) + strlen(s3) + 1) );
  strcpy(new, s1);
  strcat(new, s2);
  strcat(new, s3);
  return new;
}

char* convertTime(const time_t* mtime){
  char* buff = malloc(sizeof(char) * 30);
  struct tm * timeinfo;
  timeinfo = localtime (mtime);
  strftime(buff, 20, "%b %d %H:%M", timeinfo);
  return buff;
}

void printPermissions(mode_t mode){
  printf( (mode & S_IRUSR) ? "r" : "-");
  printf( (mode & S_IWUSR) ? "w" : "-");
  printf( (mode & S_IXUSR) ? "x" : "-");
  printf( (mode & S_IRGRP) ? "r" : "-");
  printf( (mode & S_IWGRP) ? "w" : "-");
  printf( (mode & S_IXGRP) ? "x" : "-");
  printf( (mode & S_IROTH) ? "r" : "-");
  printf( (mode & S_IWOTH) ? "w" : "-");
  printf( (mode & S_IXOTH) ? "x" : "-");
  printf("\n");
}
