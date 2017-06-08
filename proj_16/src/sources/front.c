#include "../headers/front.h"

static SDL_Texture* getTexture(const char* path, SDL_Renderer* rend);

void frontEndSetup(GameDef* g){
  g->f.sizes[0].w = 60;
  g->f.sizes[0].h = 60;
  g->f.sizes[0].n = 10;

  g->f.sizes[1].w = 40;
  g->f.sizes[1].h = 40;
  g->f.sizes[1].n = 15;

  g->f.sizes[2].w = 30;
  g->f.sizes[2].h = 30;
  g->f.sizes[2].n = 20;

  g->f.sizes[3].w = 25;
  g->f.sizes[3].h = 25;
  g->f.sizes[3].n = 24;

  g->f.sizes[4].w = 20;
  g->f.sizes[4].h = 20;
  g->f.sizes[4].n = 30;

  g->f.csp = 2;

  g->f.optDest.w = OPTION_WIDTH;
  g->f.optDest.h = OPTION_HEIGHT;
  g->f.optDest.x = g->f.sizes[g->f.csp].n * g->f.sizes[g->f.csp].w;

  g->f.win = SDL_CreateWindow("TicTacToe!",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,WINDOW_WIDTH, WINDOW_HEIGHT,0);
  if(g->f.win == NULL) SDL_throw("creating main window");

  g->f.rend = SDL_CreateRenderer(g->f.win, -1, SDL_RENDERER_ACCELERATED);
  if (!g->f.rend) SDL_throw("creating renderer");

  getTextures(g);

  WaitForSingleObject(g->b.mutex, INFINITE);
  refreshView(g);
  ReleaseMutex(g->b.mutex);
}

void refreshView(GameDef* g){
  g->f.optDest.y = 0;
  if(g->b.turn == 1){
    SDL_RenderCopy(g->f.rend, g->f.texYourMove, NULL, &g->f.optDest);
  }else{
    SDL_RenderCopy(g->f.rend, g->f.texEnemyMove, NULL, &g->f.optDest);
  }
  g->f.optDest.y += OPTION_HEIGHT;
  switch(g->b.status){
    case LOST: SDL_RenderCopy(g->f.rend, g->f.texLost, NULL, &g->f.optDest);break;
    case GOING: SDL_RenderCopy(g->f.rend, g->f.texGoing, NULL, &g->f.optDest);break;
    case WON: SDL_RenderCopy(g->f.rend, g->f.texWon, NULL, &g->f.optDest);break;
    default: break;
  }
  if(g->b.isHost == 1){
    g->f.optDest.y += OPTION_HEIGHT;
    SDL_RenderCopy(g->f.rend, g->f.texReset, NULL, &g->f.optDest);
  }

  g->f.dest.w = g->f.sizes[g->f.csp].w;
  g->f.dest.h = g->f.sizes[g->f.csp].h;

  drawMap(g);
}

void drawMap(GameDef* g){
  for(int i=0; i<g->f.sizes[g->f.csp].n; i++){
    for(int j=0; j<g->f.sizes[g->f.csp].n; j++){
      int x = g->f.viewX + i - g->f.sizes[g->f.csp].n / 2;
      int y = g->f.viewY + j - g->f.sizes[g->f.csp].n / 2;

      SDL_Texture* toDraw;
      switch(g->b.map[x][y]){
        case (2*CIRCLE): toDraw = g->f.texCircleRed; break;
        case (CIRCLE): toDraw = g->f.texCircle; break;
        case 0: toDraw = g->f.texEmpty; break;
        case CROSS: toDraw = g->f.texCross; break;
        case (2*CROSS): toDraw = g->f.texCrossRed; break;
        case BURN: toDraw = g->f.texBurn; break;
        default: toDraw = g->f.texEmpty; break; // textura bledu!
      }

      g->f.dest.x = i * g->f.sizes[g->f.csp].w;
      g->f.dest.y = j * g->f.sizes[g->f.csp].h;

      SDL_RenderCopy(g->f.rend, toDraw, NULL, &g->f.dest);
    }
  }
  SDL_RenderPresent(g->f.rend);
}

void changeView(GameDef* g, int x, int y){
  WaitForSingleObject(g->b.mutex, INFINITE);
  if(g->b.connected == 1){
    if(((g->f.viewX + g->f.sizes[g->f.csp].n + x - g->f.sizes[g->f.csp].n / 2) <= MAP_SIZE) && (g->f.viewX + x - g->f.sizes[g->f.csp].n / 2 >= 0)){
      g->f.viewX += x;
    }
    if(((g->f.viewY + g->f.sizes[g->f.csp].n + y - g->f.sizes[g->f.csp].n / 2) <= MAP_SIZE) && ((g->f.viewY + y - g->f.sizes[g->f.csp].n / 2) >= 0)){
      g->f.viewY += y;
    }
  }
  refreshView(g);
  ReleaseMutex(g->b.mutex);
}

void getTextures(GameDef* g){
  g->f.texEmpty = getTexture("resources/empty.png", g->f.rend);
  g->f.texCircle = getTexture("resources/circle.png", g->f.rend);
  g->f.texCircleRed = getTexture("resources/circleRed.png", g->f.rend);
  g->f.texCross = getTexture("resources/cross.png", g->f.rend);
  g->f.texCrossRed = getTexture("resources/crossRed.png", g->f.rend);
  g->f.texBurn = getTexture("resources/burn.png", g->f.rend);
  g->f.texUndef = getTexture("resources/undef.png", g->f.rend);
  g->f.texYourMove = getTexture("resources/yourMove.png", g->f.rend);
  g->f.texEnemyMove = getTexture("resources/enemyMove.png", g->f.rend);
  g->f.texLost = getTexture("resources/lost.png", g->f.rend);
  g->f.texGoing = getTexture("resources/going.png", g->f.rend);
  g->f.texWon = getTexture("resources/won.png", g->f.rend);
  g->f.texReset = getTexture("resources/reset.png", g->f.rend);
}

static SDL_Texture* getTexture(const char* path, SDL_Renderer* rend){
  SDL_Surface* surface = IMG_Load(path);
  if (surface == NULL) SDL_throw("preparing surface");

  SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surface);
  SDL_FreeSurface(surface);
  if (tex == NULL) SDL_throw("preparing texture");

  return tex;
}
