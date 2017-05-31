#ifndef HELPERS_H
#define HELPERS_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#define tabSize 1000

char* concat(const char *s1, const char *s2);
char* concat3(const char *s1, const char *s2, const char* s3);
char* popFragment(char** origStr, char delim);
char* trimWhite(char* origStr);
void throw(const char* err);
char* convertTime(const time_t* mtime);
int getQID(char* path, int ID);
long getMicroTime();
void swap(int *a, int *b);
void slower(int n);
int* getRandomTable(int size);

typedef struct Client{
  int fd;
  char* name;
  char sent;
  char rec;
}Client;

typedef enum messTypes{
  LOGIN = 0, RESULT = 1, FAILSIZE = 2,
  FAILNAME = 3, PING = 4, PRES = 5,
  REQ = 6
}messTypes;

#endif
