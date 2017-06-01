#ifndef HELPERS_H
#define HELPERS_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#define UNIX_PATH_MAX 108
#define ASIZE sizeof(struct sockaddr)

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
  struct sockaddr* addr;
  socklen_t aSize;
  char type;
  char* name;
  char sent;
  char rec;
}Client;

typedef struct Message{ // dla datagramow wysylam wszystko razem, zeby sie nie mieszaly
  char type;
  char name[21];
  char connectType;
  int ctn;
  int result;
}Message;

typedef enum messTypes{
  LOGIN = 0, RESULT = 1, FAILSIZE = 2,
  FAILNAME = 3, PING = 4, PONG = 5,
  REQ = 6, SUCCESS = 7, LOGOUT = 8
}messTypes;

typedef enum connectTypes{
  LOCAL = 0, WEB = 1
}connectTypes;

#endif
