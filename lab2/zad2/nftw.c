#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <ftw.h>
#include <linux/limits.h>

#include "helpers.h"

int maxSize;

int showFile(const char *path, const struct stat* sb, int typeflag, struct FTW *ftwbuf){
  if((typeflag == FTW_F) && (sb->st_size <= maxSize)){
    char* timeBuff = convertTime(&(sb->st_mtime));
    printf("%s, size: %ld, last modified: %s per: ", path, sb->st_size, timeBuff);
    printPermissions(sb->st_mode);
    free(timeBuff);
  }
  return 0;
}

void showFiles(char* path){
  if(nftw(path, showFile, 20, FTW_PHYS) == -1) {
    printf("Nftw reports error!.\n");
    return;
  }
}

int main(int argc, char** argv){
  if(argc != 3){
    printf("Wrong number of arguments!\n");
    return -1;
  }

  char realPathBuff[PATH_MAX+1];
  char* realPath = realpath(argv[1], realPathBuff);
  if(realPath == NULL){
    printf("Error reading your path! \n");
    return 1;
  }

  maxSize = atoi(argv[2]);
  if((maxSize < 0) || (maxSize > 1000000000)){
    printf("Wrong max size!\n");
    return 1;
  }
  showFiles(realPath);
}
