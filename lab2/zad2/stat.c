#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <errno.h>
#include <time.h>

#include "helpers.h"

void showFiles(char* path, int size){
  DIR* dir;
  struct dirent* file;

  dir = opendir(path);
  if(!dir){
    printf("Couln't open directory: %s\n", path);
    return;
  }

  file = readdir(dir);
  while( file != NULL ){
    if(file->d_type == DT_DIR){

      if ( (strcmp(file->d_name, ".") != 0) && (strcmp(file->d_name, "..") != 0) ){
        char* newPath = concat3(path, "/", file->d_name);
        showFiles(newPath, size);
        free(newPath);
      }

    }else if(file->d_type == DT_REG){
      struct stat* tmp = malloc(sizeof(struct stat));
      char* newPath = concat3(path, "/", file->d_name);

      if(lstat(newPath, tmp) != 0){
        printf("Error reading file %s\n", file->d_name);
        return;
      }
      if(tmp->st_size <= size){
        char* timeBuff = convertTime(&(tmp->st_mtime));
        printf("%s, size: %ld, last modified: %s per: ", newPath, tmp->st_size, timeBuff);
        printPermissions(tmp->st_mode);
        free(timeBuff);
      }

      free(tmp);
      free(newPath);
    }
    file = readdir(dir);
  }

  closedir(dir);
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

  int maxSize = atoi(argv[2]);
  if((maxSize < 0) || (maxSize > 1000000000)){
    printf("Wrong max size!\n");
    return 1;
  }

  showFiles(realPath, maxSize);
}
