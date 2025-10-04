#include <windows.h>
#include "globals.h"
#include "renderer.h"
#include "game_logic.h"
#include "sound.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;
    BOOL bQuit = FALSE;

    // Register the window class
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WindowProc; // Use WindowProc from game_logic
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "SmallCraft50KWindowClass";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Create the window
    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE, "SmallCraft50KWindowClass", "SmallCraft 50K",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // --- Memory and Initialization ---
    // Memory limit set to 200MB as requested
    const SIZE_T MEMORY_SIZE = 200 * 1024 * 1024;
    worldData = (char*)VirtualAlloc(NULL, MEMORY_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (worldData == NULL) {
        MessageBox(NULL, "Memory Allocation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    if ((long long)WORLD_WIDTH * WORLD_HEIGHT * WORLD_DEPTH > MEMORY_SIZE) {
         MessageBox(NULL, "World size exceeds allocated memory!", "Error!", MB_ICONEXCLAMATION | MB_OK);
         VirtualFree(worldData, 0, MEM_RELEASE);
         return 0;
    }

    EnableOpenGL(hwnd);
    InitializeGame();
    SetCursorPos(400, 300); // Center cursor after init
    StartMusicThread();

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // --- Main Game Loop ---
    while (!bQuit) {
        if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) {
            if (Msg.message == WM_QUIT) {
                bQuit = TRUE;
            } else {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
        } else {
            UpdateGame();
            UpdatePhysics();
            Render();
        }
    }

    // --- Cleanup ---
    StopMusicThread();
    DisableOpenGL(hwnd);
    VirtualFree(worldData, 0, MEM_RELEASE);

    return Msg.wParam;
}