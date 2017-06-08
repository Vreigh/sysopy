#ifndef HELPERS_H
#define HELPERS_H
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#include "../headers/settings.h"

void throw(const char* err);
void SDL_throw(const char* err);
char* concat(const char* one, const char* two);
void readAddress(struct sockaddr_in* server);
char trimName(char* name);
char* getSummary(char* victor, char vIcon, char* lost, char lIcon);

typedef enum player_icon{
  CIRCLE = -1, CROSS = 1, BURN = 5
}player_icon;

typedef enum game_status{
   LOST = -1, GOING = 0, WON = 1
}game_status;

typedef enum win_how{
  X = 0, Y = 1, BOTH = 2, ALT = 3
}win_how;

typedef struct GameInfo{
  char icon; // ikona ktora klient bedzie uzywac
  char turn; // czy tura klienta?
  char status;
  int lastX;
  int lastY;
  int fieldN; // ile nastepnie zostanie wyslanych pól
}GameInfo;

typedef struct TttField{
  short x; // short dla skrocenia wiadomosci
  short y;
  char icon;
}TttField;

typedef struct Move{
  short x; // short dla skrocenia wiadomosci
  short y;
  char win;
}Move;

typedef struct MapSize{
  short w;
  short h;
  short n;
}MapSize;

typedef struct BackDef{
  HANDLE listener;
  HANDLE mutex;

  struct sockaddr_in server;
  SOCKET sockServer; //= INVALID_SOCKET;
  SOCKET sockClient; //= INVALID_SOCKET;

  char icon;
  char turn;
  char status;
  char map[MAP_SIZE][MAP_SIZE];

  char isHost;
  char connected; // = 0

  char myName[50];
  char otherName[50];

  int lastX; // =0
  int lastY; // = 0

}BackDef;

typedef struct FrontDef{
  SDL_Window* win;
  SDL_Renderer* rend;

  SDL_Texture* texEmpty;
  SDL_Texture* texCircle;
  SDL_Texture* texCircleRed;
  SDL_Texture* texCross;
  SDL_Texture* texCrossRed;
  SDL_Texture* texBurn;
  SDL_Texture* texUndef;
  SDL_Texture* texYourMove;
  SDL_Texture* texEnemyMove;

  SDL_Texture* texLost;
  SDL_Texture* texGoing;
  SDL_Texture* texWon;
  SDL_Texture* texReset;

  SDL_Rect dest; // struktura opisujaca textury icon
  SDL_Rect optDest; // struktura opisujaca textury tekstu

  int viewX;
  int viewY;
  unsigned int csp;

  MapSize sizes[5];

}FrontDef;

typedef struct GameDef{
  BackDef b;
  FrontDef f;
}GameDef;

#endif
