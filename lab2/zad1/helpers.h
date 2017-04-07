#ifndef _HELPERS_H_
#define _HELPERS_H_

int checkErrors(int* ioEr, int ioN, int size, int* othEr, int othN, int isSystem, char* desc);

int cloneFile(char* orig, char* dest, int number, int size);

#endif
