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
char* concat3(const char *s1, const char *s2, const char* s3){
  char* new = malloc(sizeof(char) * (strlen(s1) + strlen(s2) + strlen(s3) + 1) );
  strcpy(new, s1);
  strcat(new, s2);
  strcat(new, s3);
  return new;
}

char* popFragment(char** origStr, char delim){
  if(*origStr == NULL) return NULL; // zwracam NULL jesli nie ma juz z czego popowac

  char* i = *origStr;
  while(*i != delim){
    i++;
    if(*i == 0){
      char* ret = *origStr;
      *origStr = NULL;
      return ret;
    }
  }

  if(i == *origStr){
    char* tmp = concat("", i + 1);
    free(*origStr);
    *origStr = tmp;

    char* ret = malloc(sizeof(char));
    *ret = 0;
    return ret;
  }

  *i = 0;
  char* ret = concat("", *origStr);
  if(i[1] != 0){
    char* tmp = concat("", i + 1);
    free(*origStr);
    *origStr = tmp;
  }else{
    free(*origStr);
    *origStr = NULL;
  }

  return ret;
}
