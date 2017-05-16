#ifndef HELPERS_H
#define HELPERS_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

char* concat(const char *s1, const char *s2);
char* concat3(const char *s1, const char *s2, const char* s3);
char* popFragment(char** origStr, char delim);
char* trimWhite(char* origStr);
void throw(const char* err);
char* convertTime(const time_t* mtime);
int getQID(char* path, int ID);
long getMicroTime();

typedef struct Record{
  int id;
  char text[1021];
}Record;

#endif
