#ifndef THREADS_H
#define THREADS_H

#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <time.h>

#include "../headers/helpers.h"
#include "../headers/settings.h"
#include "../headers/back.h"
#include "../headers/front.h"
#include "../headers/handler.h"

#pragma comment(lib,"ws2_32.lib") //Winsock Library

DWORD WINAPI hostListenerFunc(void* arg);
DWORD WINAPI clientListenerFunc(void* arg);

#endif
