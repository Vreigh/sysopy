#include "../headers/threads.h"

static void cleanup(void);
static void initialize();

GameDef g;

int main(int argc, char** argv){
  if(atexit(cleanup) != 0) throw("atexit failed!");

  initialize();
  backEndSetup(&g); // przygotuj sie backendowo, niech klient/serwer ma juz gotowa mape
  frontEndSetup(&g); // gdy backend gotowy, utworz okienko i odswiez widok

  while(1){
    SDL_Event event;
    while (SDL_PollEvent(&event)){
      if(event.type == SDL_QUIT) return 0;
      else if(event.type == SDL_KEYUP){
        switch(event.key.keysym.scancode){
          case SDL_SCANCODE_UP: changeView(&g, 0, -1); break;
          case SDL_SCANCODE_DOWN: changeView(&g, 0, 1); break;
          case SDL_SCANCODE_LEFT: changeView(&g, -1, 0); break;
          case SDL_SCANCODE_RIGHT: changeView(&g, 1, 0); break;
          default: break;
        }
      }else if(event.type == SDL_MOUSEBUTTONUP){
        if(event.button.button == SDL_BUTTON_LEFT){
          handleMouseClick(&g, event.button.x, event.button.y);
        }
      }else if(event.type == SDL_MOUSEWHEEL){
        handleScroll(&g, event.wheel.y);
      }
    }
  }
  return 0;
}

DWORD WINAPI hostListenerFunc(void* arg){
  while(1){
    printf("Enter port number!\n");
    short port;
    scanf("%hd", &port);

    g.b.server.sin_addr.s_addr = INADDR_ANY;
    g.b.server.sin_family = AF_INET;
    g.b.server.sin_port = htons(port);

    if(bind(g.b.sockServer , (struct sockaddr *)&g.b.server , sizeof(g.b.server)) == 0) break;
    printf("Couldnt bind to port you entered! Try again! Error: %d\n", WSAGetLastError());
  }

  if(listen(g.b.sockServer, 5) == SOCKET_ERROR) throw("Error listening to connections!\n");
  puts("Waiting for incoming connections...");

  while(1){
    g.b.sockClient = accept(g.b.sockServer, NULL, NULL);
    if(g.b.sockClient == INVALID_SOCKET){
      printf("Accept failed! Contineuing...\n");
      continue;
    }

    printf("Player connected!\n");

    if(exchangeNames(g.b.sockClient, g.b.myName, g.b.otherName) != 0){
      printf("Getting client name failed, looking for another one...\n");
      if(closesocket(g.b.sockClient) == SOCKET_ERROR) printf("Error closing sockClient\n");
      g.b.sockClient = INVALID_SOCKET;
      continue;
    }

    if(pushGameToClient(&g) == -1){
      printf("Sending data to client failed, looking for another one...\n");
      if(closesocket(g.b.sockClient) == SOCKET_ERROR) printf("Error closing sockClient\n");
      g.b.sockClient = INVALID_SOCKET;
      continue;
    }

    Move info;
    while(1){
      if(recv(g.b.sockClient, (char*)&info, sizeof(Move), 0) != sizeof(Move)){
        WaitForSingleObject(g.b.mutex, INFINITE);
        printf("Client connection lost!\n");

        g.b.connected = 0;
        if(closesocket(g.b.sockClient) == SOCKET_ERROR) printf("Error closing sockClient\n");
        g.b.sockClient = INVALID_SOCKET;
        ReleaseMutex(g.b.mutex);
        break;
      }

      int x = (int)ntohs(info.x);
      int y = (int)ntohs(info.y);
      enemyMove(&g, x, y, info.win);
    }
  }
}

DWORD WINAPI clientListenerFunc(void* arg){
  Move info; //narazie zakladam, ze serwer wysyla tylko informacje o ruchu (moglby jeszcze o zmiane stanu gry)
  while(1){
    if(recv(g.b.sockServer, (char*)&info, sizeof(Move), 0) != sizeof(Move)){
      throw("Server connection lost while awaiting for move info!\n");
    }

    int x = (int)ntohs(info.x);
    int y = (int)ntohs(info.y);
    if(x == MAP_SIZE) resetGame(&g, info.win);
    else enemyMove(&g, x, y, info.win);
  }
}

static void initialize(){
  WSADATA wsa; // inicjalizacja socketow
  printf("\nInitialising Winsock...");
  if (WSAStartup(MAKEWORD(2,2),&wsa) != 0){
      printf("Failed. Error Code : %d",WSAGetLastError());
      exit(1);
  }printf("Initialised.\n");

  g.f.win = NULL;
  g.f.rend = NULL;
  g.f.texEmpty = NULL;
  g.f.texCircle = NULL;
  g.f.texCircleRed = NULL;
  g.f.texCross = NULL;
  g.f.texCrossRed = NULL;
  g.f.texBurn = NULL;
  g.f.texUndef = NULL;
  g.f.texYourMove = NULL;
  g.f.texEnemyMove = NULL;

  g.f.texLost = NULL;
  g.f.texGoing = NULL;
  g.f.texWon = NULL;
  g.f.texReset = NULL;

  g.f.viewX = INITIAL_VIEW;
  g.f.viewY = INITIAL_VIEW;
  g.f.csp = 2;

  g.b.sockServer = INVALID_SOCKET;
  g.b.sockClient = INVALID_SOCKET;

  g.b.connected = 0;
  g.b.lastX = 0;
  g.b.lastY = 0;
}

static void cleanup(){
  if(g.b.sockServer != INVALID_SOCKET){
    if(closesocket(g.b.sockServer) == SOCKET_ERROR) printf("Error closing sockServer\n");
  }
  if(g.b.sockClient != INVALID_SOCKET){
    if(closesocket(g.b.sockClient) == SOCKET_ERROR) printf("Error closing sockClient\n");
  }

  if(g.f.win != NULL) SDL_DestroyWindow(g.f.win);
  if(g.f.rend != NULL) SDL_DestroyRenderer(g.f.rend);

  if(g.f.texEmpty != NULL) SDL_DestroyTexture(g.f.texEmpty);
  if(g.f.texCircle != NULL) SDL_DestroyTexture(g.f.texCircle);
  if(g.f.texCircleRed != NULL) SDL_DestroyTexture(g.f.texCircleRed);
  if(g.f.texCross != NULL) SDL_DestroyTexture(g.f.texCross);
  if(g.f.texCrossRed != NULL) SDL_DestroyTexture(g.f.texCrossRed);
  if(g.f.texBurn != NULL) SDL_DestroyTexture(g.f.texBurn);
  if(g.f.texUndef != NULL) SDL_DestroyTexture(g.f.texUndef);
  if(g.f.texYourMove != NULL) SDL_DestroyTexture(g.f.texYourMove);
  if(g.f.texEnemyMove != NULL) SDL_DestroyTexture(g.f.texEnemyMove);
  if(g.f.texLost != NULL) SDL_DestroyTexture(g.f.texLost);
  if(g.f.texGoing != NULL) SDL_DestroyTexture(g.f.texGoing);
  if(g.f.texWon != NULL) SDL_DestroyTexture(g.f.texWon);
  if(g.f.texReset != NULL) SDL_DestroyTexture(g.f.texReset);

  if(g.b.mutex != NULL){
    if(CloseHandle(g.b.mutex) == 0) printf("Error releasing mutex\n");
  }

  SDL_Quit();
}
