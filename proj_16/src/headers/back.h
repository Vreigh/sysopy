#ifndef HANDLER_H
#define HANDLER_H
#include "../headers/threads.h"

void backEndSetup(GameDef* g);
void createNewGame(GameDef* g);
void connectToGame(GameDef* g);
char exchangeNames(SOCKET s, char* myName, char* otherName);
void loadGameFromServer(GameDef* g);
char pushGameToClient(GameDef* g);
void resetGame(GameDef* g, char newIcon);
void reqResetGame(GameDef* g);
void makeMove(GameDef* g, int x, int y); //
void enemyMove(GameDef* g, int x, int y, char win); //
void clearMap(char map[MAP_SIZE][MAP_SIZE]);


#endif
