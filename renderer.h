#ifndef RENDERER_H
#define RENDERER_H

#include <windows.h>

void EnableOpenGL(HWND hwnd);
void DisableOpenGL(HWND hwnd);
void Render();
void RenderTitleScreen();
void RenderSettingsScreen();
void RenderWorldSelectScreen();
void RenderCraftingScreen();
void RenderHotbar();

#endif // RENDERER_H