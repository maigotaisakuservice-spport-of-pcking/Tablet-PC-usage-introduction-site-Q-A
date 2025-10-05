#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <windows.h>
#include "character.h"
#include "character_renderer.h"


void InitializeGame();
void UpdateGame();
void UpdatePhysics();
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // GAME_LOGIC_H