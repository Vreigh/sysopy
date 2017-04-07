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

#include "helpers.h"
#include "sys.h"
#include "lib.h"

void compare(int number, int size);
void tStart(struct tms* usTime, clock_t* rTime);
void tStop(struct tms* usTime, clock_t* rTime);

int main(int argc, char** argv){ // sprawdzenie poprawnosci wejscia, wywolanie odpowiedniego zadania
  srand(time(NULL));   // should only be called once
  char* SYS_FLAG = "sys";
  char* LIB_FLAG = "lib";
  char* CMP_FLAG = "cmp";

  char* GEN_FLAG = "generate";
  char* SHUFF_FLAG = "shuffle";
  char* SORT_FLAG = "sort";

  if((argc == 4) && (strcmp(argv[1], CMP_FLAG) == 0)){
    int number = atoi(argv[2]);
    int size = atoi(argv[3]);
    if(number < 0 || size < 0){
      printf("Wrong size or number of records! \n"); return 1;
    }
    compare(number, size);
    return 0;
  }

  if(argc < 6){
    printf("Not enough arguments! \n");
    return 1;
  }

  int number = atoi(argv[4]);
  int size = atoi(argv[5]);

  if(number < 0 || size < 0){
    printf("Wrong size or number of records! \n"); return 1;
  }

  if((strcmp(argv[2], GEN_FLAG) != 0) && (access(argv[3], F_OK) == -1)){
    printf("Wrong filename! \n"); return 1;
  }

  if(strcmp(argv[1], SYS_FLAG) == 0){
    if(strcmp(argv[2], GEN_FLAG) == 0){
      return generateSys(argv[3], number, size);
    }else if(strcmp(argv[2], SHUFF_FLAG) == 0){
      return shuffleSys(argv[3], number, size);
    }else if(strcmp(argv[2], SORT_FLAG) == 0){
      return sortSys(argv[3], number, size);
    }else{
      printf("Wrong command!\n"); return 1;
    }
  }else if(strcmp(argv[1], LIB_FLAG) == 0){
    if(strcmp(argv[2], GEN_FLAG) == 0){
      return generateLib(argv[3], number, size);
    }else if(strcmp(argv[2], SHUFF_FLAG) == 0){
      return shuffleLib(argv[3], number, size);
    }else if(strcmp(argv[2], SORT_FLAG) == 0){
      return sortLib(argv[3], number, size);
    }else{
      printf("Wrong command!\n"); return 1;
    }
  }else{
    printf("Wrong implementation mode! \n");
    return 1;
  }
  return 0;
}

void compare(int number, int size){
  struct tms* usTime = malloc(sizeof(struct tms));
  clock_t* rTime = malloc(sizeof(clock_t));

  char* tmp = "tmp.txt";
  char* tmp1 = "tmp1.txt";

  printf("Generating new file %d X %d\n", number, size);
  tStart(usTime, rTime);
  if(generateSys(tmp, number, size) == 1) return;
  tStop(usTime, rTime);


  printf("System shufflig file %d X %d\n", number, size);
  tStart(usTime, rTime);
  if(shuffleSys(tmp, number, size) == 1) return;
  tStop(usTime, rTime);


  printf("Library shufflig file %d X %d\n", number, size);
  tStart(usTime, rTime);
  if(shuffleLib(tmp, number, size) == 1) return;
  tStop(usTime, rTime);

  if(cloneFile(tmp, tmp1, number, size) == 1) return;

  printf("System sorting file %d X %d\n", number, size);
  tStart(usTime, rTime);
  if(sortSys(tmp, number, size) == 1) return;
  tStop(usTime, rTime);

  printf("Library sorting file %d X %d\n", number, size);
  tStart(usTime, rTime);
  if(sortLib(tmp1, number, size) == 1) return;
  tStop(usTime, rTime);

  printf("-------------------------------------------------------------------------\n \n");

}

#define CLK sysconf(_SC_CLK_TCK)

void tStart(struct tms* usTime, clock_t* rTime){
  *rTime = clock();
  times(usTime);
}

void tStop(struct tms* usTime, clock_t* rTime){
  clock_t rNow = clock();
  struct tms usNow;
  times(&usNow);

  double difReal = ((double)(rNow - *rTime)) / CLOCKS_PER_SEC;
  double difUser = ((double) (usNow.tms_utime - usTime->tms_utime)) / CLK;
  double difSys = ((double)(usNow.tms_stime - usTime->tms_stime)) / CLK;

  printf("\n Time of operation:\tReal: %.6f\tUser: %.6f\tSystem: %.6f \n \n", difReal, difUser, difSys);
}
