#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <sys/wait.h>
#include <errno.h>

#include "helpers.h"

void createEnv(char*);
void createProc(char*, rlim_t, rlim_t);
char* takeOnlyName(const char*);
void showUsage(){
  printf("Use program like ./exec <filename> <timeLimit> <memoryLimit>\n");
}

int main(int argc, char** argv)
{
  if(argc != 4){showUsage();return 1;}

  rlim_t tLim = atoi(argv[2]);
  rlim_t mLim = atoi(argv[3]) * 1024 * 1024;
  if(tLim < 1 || mLim < 100){
    printf("Wrong limits!\n");
    return 1;
  }

  const char* inputPath = argv[1];
  FILE* dsc = fopen(inputPath, "r");
  if(dsc == NULL){
    printf("Error reading Your file!\n");
    return 2;
  }

  char line[100];
  while (fgets(line, sizeof(line), dsc)) {
    int len = strlen(line);
    line[len - 1] = 0;
    if(line[0] == '#'){
      createEnv(line + 1);
    }else{
      createProc(line, tLim, mLim);
    }
  }
  fclose(dsc);
  return 0;
}

void createEnv(char* line){
  char* val = strtok(line, " ");
  if(val == NULL){
    printf("Wrong format of enviromental variable line!\n"); return;
  }
  char key[strlen(val) + 1];
  strcpy(key, val);
  val = strtok(NULL, " ");

  if(val != NULL){
    printf("Setting enviromental variable: %s with value of: %s\n", key, val);
    if(setenv(key, val, 1) == -1){
      printf("*** Error setting enviromental variable! ***\n");
      return;
    }
  }else{
    printf("Unsetting enviromental variable: %s\n", key);
    if(unsetenv(key) == -1){
      printf("*** Error deleting enviromental variable! ***\n");
      return;
    }
  }
  printf("----------------------------------------------------------\n");
}
void createProc(char* line, rlim_t tLim, rlim_t mLim){
  char* lineCp = concat("", line);
  int j = 0;
  char* i = strtok(lineCp, " ");
  if(i == NULL){
    printf("Wrong format of procedure line!\n"); return;
  }
  while(i != NULL){
    j++; i = strtok(NULL, " ");
  }
  char* args[j+1];
  j = 0;

  i = strtok(line, " ");
  while(i != NULL){
    args[j++] = concat("", i);
    i = strtok(NULL, " ");
  }
  args[j] = NULL;

  pid_t fork_0 = fork();
  if(fork_0 == -1){
    printf("Error creating child process!");
    exit(3);
  }
  else if(fork_0 == 0){
    printf("Executing command: %s\n",args[0]);

    struct rlimit timeLimit;
    timeLimit.rlim_cur = tLim;
    timeLimit.rlim_max = tLim;

    struct rlimit memLimit;
    memLimit.rlim_cur = mLim;
    memLimit.rlim_max = mLim;

    if(setrlimit(RLIMIT_CPU, &timeLimit) == -1){
      printf("Error setting time limit for task: %s due to errno: %d\n", args[0], errno);
    }
    if(setrlimit(RLIMIT_AS, &memLimit) == -1){
      printf("Error setting memory limit for task: %s due to errno: %d\n", args[0], errno);
    }

    execvp(args[0], args);

    printf("Couldnt execute command %s due to errno %d!\n", args[0], errno);
    exit(4);
  }else{
    int status;
    struct rusage use;
    wait4(fork_0,&status,0, &use);
    if(WIFEXITED(status) && (WEXITSTATUS(status) != 0)){
      printf("*** Process of command %s reported error, abording! *** \n", args[0]);
      exit(5);
    }
    printf("----------\n");
    printf("System time: %ld\n", use.ru_stime.tv_usec);
    printf("User time: %ld\n", use.ru_utime.tv_usec);
    printf("Maximum set size: %ld\n", use.ru_maxrss);
    printf("Integral shared memory size: %ld\n", use.ru_ixrss);
    printf("----------------------------------------------------------\n");
  }
}

/*char* takeOnlyName(const char* path){
  char* pathCp = concat("", path);
  //char* pathCp = malloc(strlen(path) + 1);
  //strcpy(pathCp, path);
  char* name = pathCp;
  char* i = strtok(pathCp, "/");
  while(i != NULL){
    name = i;
    i = strtok(NULL, "/");
  }
  return name;
}*/
