#ifndef HELPERS_H
#define HELPERS_H
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

char* concat(const char *s1, const char *s2);
char* concat3(char *s1, char *s2, char* s3);
char* convertTime(const time_t* mtime);
void printPermissions(mode_t mode);

#endif
