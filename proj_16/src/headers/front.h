#ifndef HANDLER_H
#define HANDLER_H
#include "../headers/threads.h"

void frontEndSetup(GameDef* g);
void refreshView(GameDef* g);
void drawMap(GameDef* g);
void changeView(GameDef* g, int x, int y);
void getTextures(GameDef* g);

#endif
