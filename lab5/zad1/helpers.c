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
  if(*origStr == NULL) return NULL; // zwraca NULL jesli nie ma juz z czego popowac

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
    *origStr = i + 1;

    char* ret = malloc(sizeof(char));
    *ret = 0;
    return ret;
  }

  *i = 0;
  char* ret = *origStr;
  if(i[1] != 0){
    *origStr = i + 1;
  }else{
    *origStr = NULL;
  }
  return ret;
}

char* trimWhite(char* origStr){
  char* buffer = malloc(sizeof(char) * 200);
  char* i = origStr;
  while(*i == ' ') i++;
  int j = 0;
  while(*i != 0){
    while((*i != ' ') && (*i != 0)){
      buffer[j++] = *i;
      i++;
    }
    if(*i == ' '){
      buffer[j++] = ' ';
      while(*i == ' ') i++;
    }
  }
  buffer[j++] = 0;
  buffer = realloc(buffer, j);
  return buffer;
}
