#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helpers.h"

char* concat(const char* one, const char* two){
    char* new = malloc(sizeof(char) * (strlen(one) + strlen(two) + 1) );
    strcpy(new, one);
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
