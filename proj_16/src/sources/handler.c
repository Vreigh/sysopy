#include "../headers/handler.h"

void handleMouseClick(GameDef* g,int pixX, int pixY){
  int relX = pixX / g->f.sizes[g->f.csp].w; // relative
  int relY = pixY / g->f.sizes[g->f.csp].h;
  if((relX < g->f.sizes[g->f.csp].n) && (relY < g->f.sizes[g->f.csp].n)) makeMove(g,
    g->f.viewX + relX - g->f.sizes[g->f.csp].n / 2,
    g->f.viewY + relY - g->f.sizes[g->f.csp].n / 2);
  else if((pixX >= g->f.optDest.x) && (pixY >= OPTION_HEIGHT * 2) && (pixY < OPTION_HEIGHT * 3) && (g->b.isHost == 1)){
    reqResetGame(g);
  }
}

void handleScroll(GameDef* g, int y){
  char sign;
  if(y < 0 ) sign = -1;
  else sign = 1;

  WaitForSingleObject(g->b.mutex, INFINITE);
  if((g->f.csp + sign >= 0) && (g->f.csp + sign < 5)){
    g->f.csp += sign;
  }
  refreshView(g);
  ReleaseMutex(g->b.mutex);
}
