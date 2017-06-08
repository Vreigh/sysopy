#include "../headers/back.h"

static void sendInfo(SOCKET s, char win, int x, int y);
static void handleEnd(GameDef* g, char vec, char win);
static char checkIfWin(int x, int y, char icon, char map[MAP_SIZE][MAP_SIZE]);
static char countPath(int startX, int startY, int x, int y, char icon, char map[MAP_SIZE][MAP_SIZE]);
static void burnPath(int startX, int startY, int x, int y, char icon, char map[MAP_SIZE][MAP_SIZE]);


void backEndSetup(GameDef* g){
  srand(time(NULL)>>2);
  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) throw("Error initializing SDL!");

  g->b.sockServer = socket(AF_INET, SOCK_STREAM, 0);
  if(g->b.sockServer == INVALID_SOCKET) throw("Error creating sockServer socket!");

  g->b.mutex = CreateMutex( NULL, FALSE, NULL);
  if(g->b.mutex == NULL) throw("mutex creation failed!\n");

  printf("Welcome to TicTacToe!\n");
  while(1){
    printf("Please enter your display name(at least 4 characters): ");
    while(fgets(g->b.myName, 50, stdin) == NULL);
    if(trimName(g->b.myName) == 0) break;
    printf("Wrong name!\n");
  }

  printf("Your name is %s\n", g->b.myName);

  printf("Type:\n\"new\" to create a new game and become a host, or\n\"con\" to connect to already existing one!\n");
  while(1){
    char buff[10];
    while(fgets(buff, 10, stdin) == NULL) printf("Error getting your input! Try again!\n");
    if(strcmp(buff, "new\n") == 0){
      createNewGame(g);
      break;
    }else if(strcmp(buff, "con\n") == 0){
      connectToGame(g);
      break;
    }else printf("Wrong command! Try again.\n");
  }

}

void createNewGame(GameDef* g){
  g->b.icon = 0;
  while(g->b.icon == 0) g->b.icon = rand()%3 - 1;
  if(g->b.icon == CIRCLE) g->b.turn = 1;
  else g->b.turn = -1;

  g->b.status = GOING;
  g->b.isHost = 1;
  clearMap(g->b.map);

  g->b.listener = CreateThread(NULL, 0, hostListenerFunc, NULL, 0, NULL);
}

void connectToGame(GameDef* g){
  g->b.isHost = 0;
  while(1){
    readAddress(&g->b.server);
    if(connect(g->b.sockServer , (struct sockaddr *)&g->b.server , sizeof(g->b.server)) == 0) break;
    printf("Couldnt connect to address you entered! Error: %d\n", WSAGetLastError());
  }
  g->b.connected = 1;

  if(exchangeNames(g->b.sockServer,g->b.myName, g->b.otherName) != 0) throw("Error exchanging names with host!\n");

  loadGameFromServer(g);

  g->b.listener = CreateThread(NULL, 0, clientListenerFunc, NULL, 0, NULL);
}

char exchangeNames(SOCKET s, char* myName, char* otherName){
  if(send(s, (char*)myName, 50, 0) != 50) return -1;
  if(recv(s, (char*)otherName, 50, 0) != 50) return -1;
  if(strcmp(myName, otherName) == 0){
    printf("You have the same names!\n");
    return 1;
  }
  printf("My oponnent name is: %s\n", otherName);
  return 0;
}

void loadGameFromServer(GameDef* g){
  WaitForSingleObject(g->b.mutex, INFINITE);
  GameInfo gameInfo;
  if(recv(g->b.sockServer, (char*)&gameInfo, sizeof(GameInfo), 0) != sizeof(GameInfo)){
    printf("Server connection lost!\n");
    throw("while reading connectToGame GameInfo response!");
  }
  int n = ntohl(gameInfo.fieldN);

  clearMap(g->b.map);

  TttField field;
  for(int i=0; i<n; i++){
    if(recv(g->b.sockServer, (char*)&field, sizeof(TttField), 0) != sizeof(TttField)){
      printf("Server connection lost!\n");
      throw("while reading connectToGame TttField response!");
    }
    short x = ntohs(field.x);
    short y = ntohs(field.y);
    g->b.map[x][y] = field.icon;
  }

  g->b.icon = gameInfo.icon;
  g->b.turn = gameInfo.turn;
  g->b.status = gameInfo.status;
  g->b.lastX = ntohl(gameInfo.lastX);
  g->b.lastY = ntohl(gameInfo.lastY);
  ReleaseMutex(g->b.mutex);
}

char pushGameToClient(GameDef* g){
  WaitForSingleObject(g->b.mutex, INFINITE);
  GameInfo info;
  int fieldN = 0;

  for(int i=0; i<MAP_SIZE; i++){
    for(int j=0; j<MAP_SIZE; j++){
      if(g->b.map[i][j] != 0) fieldN++;
    }
  }

  info.icon = -g->b.icon;
  info.turn = -g->b.turn;
  info.status = -g->b.status;
  info.lastX = htonl(g->b.lastX);
  info.lastY = htonl(g->b.lastY);
  info.fieldN = htonl(fieldN);

  if(send(g->b.sockClient, (char*)&info , sizeof(GameInfo), 0) != sizeof(GameInfo)) return -1;


  TttField field;
  for(short i=0; i<MAP_SIZE; i++){
    for(short j=0; j<MAP_SIZE; j++){
      if(g->b.map[i][j] != 0){
        field.x = htons(i);
        field.y = htons(j);
        field.icon = g->b.map[i][j];
        if(send(g->b.sockClient, (char*)&field , sizeof(TttField), 0) != sizeof(TttField)) return -1;
      }
    }
  }

  g->b.connected = 1;
  ReleaseMutex(g->b.mutex);
  return 0;
}

void reqResetGame(GameDef* g){
  WaitForSingleObject(g->b.mutex, INFINITE);
  char newIcon = 0;
  while(newIcon == 0) newIcon = rand()%3 - 1;
  resetGame(g, newIcon);

  Move info;
  info.x = htons((short)MAP_SIZE);
  info.y = htons((short)MAP_SIZE);
  info.win = -g->b.icon;

  if(g->b.connected == 1){
    if(send(g->b.sockClient, (char*)&info, sizeof(Move), 0) != sizeof(Move)){
      throw("Client connection lost while sending reset req!\n");
    }
    printf("Reset request sent!\n");
  }
  ReleaseMutex(g->b.mutex);
}

void resetGame(GameDef* g, char newIcon){
  WaitForSingleObject(g->b.mutex, INFINITE);
  g->b.icon = newIcon;
  if(g->b.icon == CIRCLE) g->b.turn = 1;
  else g->b.turn = -1;

  clearMap(g->b.map);
  g->f.viewX = INITIAL_VIEW;
  g->f.viewY = INITIAL_VIEW;
  g->f.csp = 2;
  g->b.status = GOING;

  refreshView(g);
  printf("Game has been reset!\n");
  ReleaseMutex(g->b.mutex);
}

void makeMove(GameDef* g, int x, int y){
  WaitForSingleObject(g->b.mutex, INFINITE);
  if((g->b.map[x][y] == 0) && (g->b.connected == 1) && (g->b.status == GOING) && (g->b.turn == 1)){
    g->b.map[g->b.lastX][g->b.lastY] /= 2;
    g->b.map[x][y] = g->b.icon * 2;
    g->b.lastX = x;
    g->b.lastY = y;
    g->b.turn = -1;

    char vec = checkIfWin(g->b.lastX, g->b.lastY, g->b.icon, g->b.map);
    handleEnd(g, vec, WON);
    refreshView(g);
    if(g->b.isHost == 1) sendInfo(g->b.sockClient, vec, x, y);
    else sendInfo(g->b.sockServer, vec, x, y);

  }
  ReleaseMutex(g->b.mutex);
}

static void sendInfo(SOCKET s, char win, int x, int y){
  Move info;
  info.x = htons((short)x);
  info.y = htons((short)y);
  info.win = win;
  if(send(s, (char*)&info, sizeof(Move), 0) != sizeof(Move)){
    throw("Connection lost while sending move info!\n");
  }
}

void enemyMove(GameDef* g, int x, int y, char win){
  WaitForSingleObject(g->b.mutex, INFINITE);
  g->b.map[g->b.lastX][g->b.lastY] /= 2;
  g->b.map[x][y] = -2*g->b.icon;
  g->b.lastX = x;
  g->b.lastY = y;
  g->b.turn = 1;

  handleEnd(g, win, LOST);
  refreshView(g);
  ReleaseMutex(g->b.mutex);
}

static void handleEnd(GameDef* g, char vec, char win){
  char compIcon = g->b.icon; // icon to compare to
  char* summary;
  if(win == LOST) compIcon *= -1;
  switch(vec){
    case X: burnPath(g->b.lastX, g->b.lastY, 1, 0, compIcon, g->b.map); break;
    case Y: burnPath(g->b.lastX, g->b.lastY, 0, 1, compIcon, g->b.map); break;
    case BOTH: burnPath(g->b.lastX, g->b.lastY, 1, 1, compIcon, g->b.map); break;
    case ALT: burnPath(g->b.lastX, g->b.lastY, 1, -1, compIcon, g->b.map); break;
    case -1: return; break;
  }
  g->b.status = win;

  FILE* results = fopen("results.txt", "a");
  if(results == NULL){
    printf("Error openings results file!\n");
    return;
  }
  if(win == WON){
    printf("Victory!\n");
    summary = getSummary(g->b.myName, g->b.icon, g->b.otherName, -g->b.icon);
  }
  if(win == LOST){
    printf("Defeat!\n");
    summary = getSummary(g->b.otherName, -g->b.icon, g->b.myName, g->b.icon);
  }
  printf("Saving results...\n");
  if(fwrite(summary, 1, strlen(summary) + 1, results) != strlen(summary) + 1){
    printf("Error saving results\n");
  }else printf("Results saved!\n");
  free(summary);
  if(fclose(results) == EOF) printf("Error closing results file!\n");
}

void clearMap(char map[MAP_SIZE][MAP_SIZE]){
  for(int i=0; i<MAP_SIZE; i++){
    for(int j=0; j<MAP_SIZE; j++){
      map[i][j] = 0;
    }
  }
}

static char checkIfWin(int lastX, int lastY, char icon, char map[MAP_SIZE][MAP_SIZE]){
  char totalX = 0;
  char totalY = 0;
  char totalBoth = 0;
  char totalAlt = 0;

  totalX += countPath(lastX, lastY, 1, 0, icon, map);
  if(totalX >= ROW) return X;

  totalY += countPath(lastX, lastY, 0, 1, icon, map);
  if(totalY >= ROW) return Y;

  totalBoth += countPath(lastX, lastY, 1, 1, icon, map);
  if(totalBoth >= ROW) return BOTH;

  totalAlt += countPath(lastX, lastY, 1, -1, icon, map);
  if(totalAlt >= ROW) return ALT;

  return -1;
}

static char countPath(int startX, int startY, int x, int y, char icon, char map[MAP_SIZE][MAP_SIZE]){
  int result = 1;
  for(int i=1; i<ROW; i++){
    int X = startX + i*x;
    int Y = startY + i*y;
    if((X > MAP_SIZE) || (X<0) || (Y > MAP_SIZE) || (Y < 0) || (map[X][Y] != icon)) break;
    result++;
  }
  for(int i=1; i<ROW; i++){
    int X = startX - i*x;
    int Y = startY - i*y;
    if((X > MAP_SIZE) || (X<0) || (Y > MAP_SIZE) || (Y < 0) || (map[X][Y] != icon)) break;
    result++;
  }
  return result;
}

static void burnPath(int startX, int startY, int x, int y, char icon, char map[MAP_SIZE][MAP_SIZE]){
  map[startX][startY] = BURN;
  for(int i=1; i<ROW; i++){
    int X = startX + i*x;
    int Y = startY + i*y;
    if((X > MAP_SIZE) || (X<0) || (Y > MAP_SIZE) || (Y < 0) || (map[X][Y] != icon)) break;
    map[X][Y] = BURN;
  }
  for(int i=1; i<ROW; i++){
    int X = startX - i*x;
    int Y = startY - i*y;
    if((X > MAP_SIZE) || (X<0) || (Y > MAP_SIZE) || (Y < 0) || (map[X][Y] != icon)) break;
    map[X][Y] = BURN;
  }
}
