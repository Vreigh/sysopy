#include "../headers/helpers.h"


void throw(const char* err){
  printf("Error! %s!", err);
  exit(3);
}

void SDL_throw(const char* err){
  printf("Error in %s! SLD: %s\n", err, SDL_GetError());
  exit(3);
}

char* concat(const char* one, const char* two){
    char* new = malloc(sizeof(char) * (strlen(one) + strlen(two) + 1) );
    strcpy(new, one);
    strcat(new, two);
    return new;
}

void readAddress(struct sockaddr_in* server){
  char buffer[100];
  char IP[40];
  short port;
  u_long address;

  while(1){
    printf("Enter port and IP Address of the host\n");
    while(fgets(buffer, 40, stdin) == NULL) printf("Error getting your input! Try again!\n");

    if(sscanf(buffer, "%hd %s", &port, IP) != 2){
      printf("Error scanning your input! Try again!\n");
      continue;
    }

    printf("%s %d\n", IP, port);

    address = inet_addr(IP);
    if(address == INADDR_NONE){
      printf("Wrong IP! Try again!\n");
    }else break;
  }

  server->sin_addr.s_addr = address;
  server->sin_family = AF_INET;
  server->sin_port = htons(port);
}

char trimName(char* name){
  for(int i=0; i<strlen(name); i++){
    if(((name[i] < 'a') || (name[i] > 'z')) &&
    ((name[i] < 'A') || (name[i] > 'Z')) &&
    ((name[i] < '0') || (name[i] > '9'))){
      name[i] = 0;
    }
  }
  if(strlen(name) >= 4) return 0;
  return 1;
}

char* getSummary(char* victor, char vIcon, char* lost, char lIcon){
  char* result = (char*)malloc(strlen(victor) + strlen(lost) + 50);
  char victorIcon, lostIcon;
  if(vIcon == CIRCLE){
    victorIcon = 'O';
    lostIcon = 'X';
  }else{
    victorIcon = 'X';
    lostIcon = 'O';
  }
  if(sprintf(result, "%s (%c) has Won with %s (%c)!\n", victor, victorIcon, lost, lostIcon) < 0){
    printf("sprintf error!\n");
  }
  return result;
}
