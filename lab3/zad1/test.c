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

int main(int argc, char** argv){
  /*char* buff = malloc(50);
  pid_t tmp = fork();
  pid_t pid = getpid();
  sprintf(buff, "I, %d, have written this line!\n", pid);
  write(1, buff, strlen(buff));
  if(tmp == 0){
    tmp = getppid();
    sprintf(buff, "I am a child, and my parent is: %d\n", tmp);
    write(1, buff, strlen(buff));
  }else{
    sprintf(buff, "I am a parent, and my child is: %d\n", tmp);
    write(1, buff, strlen(buff));
  }
  sleep(5);*/
  /*extern char** environ;
  setenv("mytestvariable", "mytestvalue", 1);
  setenv("mytestvariable", "mytestvalue2", 1);
  setenv("mytestvariable1", "mytestvalue2", 1);
  unsetenv("mytestvariable1");
  pid_t x = fork();
  if(x == 0){
    sleep(2);
    for(char** i=environ; *i!=NULL; i++){
      printf("%s\n", *i);
    }
  }else{
    setenv("mytestvariable1", "mytestvalue2", 1); // bedzie widac u dziecka?
    printf("Im a parent, and i aint do nothin\n");
    sleep(5);
  }*/

  /*int fork0 = fork();
  if(fork0 == -1){
    printf("Error creating child process!");
  }
  else if(fork0 == 0){
    char* args[2];
    args[0] = "ls";
    args[1] = NULL;
    execvp("ls", args);
    printf("Error executing execvp!!\n");
  }else{
    struct rusage* use = malloc(sizeof(struct rusage));
    int* status = malloc(sizeof(int));
    wait4(fork0, status, 0, use);
    printf("Child process executed correctly, with status: %d\n", *status);
    printf("System time: %ld\n", use->ru_stime.tv_usec);
  }*/
  if(argc < 2){
    printf("Not enough arguments!\n");
    return 1;
  }
  if(getenv(argv[1]) == NULL){
    printf("Couldnt find variable zmienna2!\n");
  }else printf("zmienna2 has a value of: %s\n", getenv("zmienna2"));
  
  return 0;
}
