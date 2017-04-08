#ifndef HELPERS_H
#define HELPERS_H
#include <string.h>

char* concat(const char *s1, const char *s2);
char* concat3(const char *s1, const char *s2, const char* s3);
char* popFragment(char** origStr, char delim);
char* trimWhite(char* origStr);
#endif
