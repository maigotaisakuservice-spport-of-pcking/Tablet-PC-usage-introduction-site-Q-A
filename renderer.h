#ifndef RENDERER_H
#define RENDERER_H

#include <windows.h>

void EnableOpenGL(HWND hwnd);
void DisableOpenGL(HWND hwnd);
void Render();
void RenderTitleScreen();
void RenderSettingsScreen();

// Drawing Primitives
void DrawCube(float x, float y, float z, const float* color, bool in_shadow);
void DrawSphere(float x, float y, float z, const float* color, bool in_shadow);

#endif // RENDERER_H