#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <math.h>
#include "marching_cubes.h"

// World dimensions
#define WORLD_SIZE_X 32
#define WORLD_SIZE_Y 32
#define WORLD_SIZE_Z 32

// World data
GLfloat world_data[WORLD_SIZE_X][WORLD_SIZE_Y][WORLD_SIZE_Z];

// Camera variables
float cam_x = WORLD_SIZE_X / 2.0f;
float cam_y = WORLD_SIZE_Y / 1.5f;
float cam_z = WORLD_SIZE_Z / 2.0f;
float cam_yaw = 0.0f;
float cam_pitch = 0.0f;

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void generate_world();
void render_world();
void handle_input(HWND hwnd);
void modify_terrain(bool add);

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR pCmdLine, int nCmdShow) {
    // Register the window class.
    const char CLASS_NAME[]  = "Sample Window Class";

    WNDCLASS wc = { };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    // Create the window.
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        "Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,

        NULL,       // Parent window
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL) {
        return 0;
    }

    // OpenGL setup
    HDC hdc = GetDC(hwnd);
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);
    HGLRC hglrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hglrc);

    generate_world();

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-20, 20, -20, 20, -100, 100);
    glMatrixMode(GL_MODELVIEW);


    ShowWindow(hwnd, nCmdShow);

    // Run the message loop.
    MSG msg = { };
    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            handle_input(hwnd);

            // Game loop
            glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glLoadIdentity();

            // Apply camera transformations
            glRotatef(cam_pitch, 1.0f, 0.0f, 0.0f);
            glRotatef(cam_yaw, 0.0f, 1.0f, 0.0f);
            glTranslatef(-cam_x, -cam_y, -cam_z);

            render_world();

            SwapBuffers(hdc);
        }
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hglrc);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
    UnregisterClass(CLASS_NAME, hInstance);


    return 0;
}

void generate_world() {
    for (int x = 0; x < WORLD_SIZE_X; x++) {
        for (int y = 0; y < WORLD_SIZE_Y; y++) {
            for (int z = 0; z < WORLD_SIZE_Z; z++) {
                if (y < 16) {
                    world_data[x][y][z] = 1.0f;
                } else {
                    world_data[x][y][z] = 0.0f;
                }
            }
        }
    }
}

void render_world() {
    glBegin(GL_TRIANGLES);
    for (int x = 0; x < WORLD_SIZE_X - 1; x++) {
        for (int y = 0; y < WORLD_SIZE_Y - 1; y++) {
            for (int z = 0; z < WORLD_SIZE_Z - 1; z++) {
                GRIDCELL cell;
                TRIANGLE triangles[10]; // Max 5 triangles per cell, but let's be safe

                // Set up the grid cell
                cell.p[0] = {(float)x, (float)y, (float)z};
                cell.p[1] = {(float)x + 1, (float)y, (float)z};
                cell.p[2] = {(float)x + 1, (float)y, (float)z + 1};
                cell.p[3] = {(float)x, (float)y, (float)z + 1};
                cell.p[4] = {(float)x, (float)y + 1, (float)z};
                cell.p[5] = {(float)x + 1, (float)y + 1, (float)z};
                cell.p[6] = {(float)x + 1, (float)y + 1, (float)z + 1};
                cell.p[7] = {(float)x, (float)y + 1, (float)z + 1};

                cell.val[0] = world_data[x][y][z];
                cell.val[1] = world_data[x + 1][y][z];
                cell.val[2] = world_data[x + 1][y][z + 1];
                cell.val[3] = world_data[x][y][z + 1];
                cell.val[4] = world_data[x][y + 1][z];
                cell.val[5] = world_data[x + 1][y + 1][z];
                cell.val[6] = world_data[x + 1][y + 1][z + 1];
                cell.val[7] = world_data[x][y + 1][z + 1];

                int num_triangles = Polygonise(cell, 0.5f, triangles);

                for (int i = 0; i < num_triangles; i++) {
                    // Center the world
                    float offsetX = WORLD_SIZE_X / 2.0f;
                    float offsetY = WORLD_SIZE_Y / 2.0f;
                    float offsetZ = WORLD_SIZE_Z / 2.0f;

                    glColor3f(0.8f, 0.8f, 0.8f);
                    glVertex3f(triangles[i].p[0].x - offsetX, triangles[i].p[0].y - offsetY, triangles[i].p[0].z - offsetZ);
                    glVertex3f(triangles[i].p[1].x - offsetX, triangles[i].p[1].y - offsetY, triangles[i].p[1].z - offsetZ);
                    glVertex3f(triangles[i].p[2].x - offsetX, triangles[i].p[2].y - offsetY, triangles[i].p[2].z - offsetZ);
                }
            }
        }
    }
    glEnd();
}

void handle_input(HWND hwnd) {
    // only handle input if the window is active
    if (GetForegroundWindow() != hwnd) {
        return;
    }

    // Keyboard movement
    float move_speed = 0.2f;
    float yaw_rad = cam_yaw * (3.1415926535f / 180.0f);

    if (GetAsyncKeyState('W') & 0x8000) {
        cam_x += sinf(yaw_rad) * move_speed;
        cam_z -= cosf(yaw_rad) * move_speed;
    }
    if (GetAsyncKeyState('S') & 0x8000) {
        cam_x -= sinf(yaw_rad) * move_speed;
        cam_z += cosf(yaw_rad) * move_speed;
    }
    if (GetAsyncKeyState('A') & 0x8000) {
        cam_x -= cosf(yaw_rad) * move_speed;
        cam_z -= sinf(yaw_rad) * move_speed;
    }
    if (GetAsyncKeyState('D') & 0x8000) {
        cam_x += cosf(yaw_rad) * move_speed;
        cam_z += sinf(yaw_rad) * move_speed;
    }
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        cam_y += move_speed;
    }
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
        cam_y -= move_speed;
    }

    // Mouse look
    float mouse_sensitivity = 0.2f;
    POINT p;
    RECT window_rect;
    GetWindowRect(hwnd, &window_rect);
    int center_x = window_rect.left + (window_rect.right - window_rect.left) / 2;
    int center_y = window_rect.top + (window_rect.bottom - window_rect.top) / 2;

    GetCursorPos(&p);

    if (p.x != center_x || p.y != center_y) {
        float dx = (float)(p.x - center_x);
        float dy = (float)(p.y - center_y);

        cam_yaw += dx * mouse_sensitivity;
        cam_pitch += dy * mouse_sensitivity;

        if (cam_pitch > 89.0f) cam_pitch = 89.0f;
        if (cam_pitch < -89.0f) cam_pitch = -89.0f;

        SetCursorPos(center_x, center_y);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE) {
                ShowCursor(TRUE);
            } else {
                ShowCursor(FALSE);
                RECT window_rect;
                GetWindowRect(hwnd, &window_rect);
                int center_x = window_rect.left + (window_rect.right - window_rect.left) / 2;
                int center_y = window_rect.top + (window_rect.bottom - window_rect.top) / 2;
                SetCursorPos(center_x, center_y);
            }
            return 0;

        case WM_LBUTTONDOWN:
            modify_terrain(true);
            return 0;

        case WM_RBUTTONDOWN:
            modify_terrain(false);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void modify_terrain(bool add) {
    float reach = 8.0f;
    float step = 0.1f;

    float yaw_rad = cam_yaw * (3.1415926535f / 180.0f);
    float pitch_rad = cam_pitch * (3.1415926535f / 180.0f);

    float dir_x = sinf(yaw_rad) * cosf(pitch_rad);
    float dir_y = -sinf(pitch_rad);
    float dir_z = -cosf(yaw_rad) * cosf(pitch_rad);

    for (float dist = 0.0f; dist < reach; dist += step) {
        float check_x = cam_x + dir_x * dist;
        float check_y = cam_y + dir_y * dist;
        float check_z = cam_z + dir_z * dist;

        int world_x = (int)roundf(check_x);
        int world_y = (int)roundf(check_y);
        int world_z = (int)roundf(check_z);

        if (world_x < 1 || world_x >= WORLD_SIZE_X - 1 ||
            world_y < 1 || world_y >= WORLD_SIZE_Y - 1 ||
            world_z < 1 || world_z >= WORLD_SIZE_Z - 1) {
            continue;
        }

        if (world_data[world_x][world_y][world_z] > 0.5f) {
            if (add) {
                float place_x = cam_x + dir_x * (dist - step * 2.0f);
                float place_y = cam_y + dir_y * (dist - step * 2.0f);
                float place_z = cam_z + dir_z * (dist - step * 2.0f);
                world_x = (int)roundf(place_x);
                world_y = (int)roundf(place_y);
                world_z = (int)roundf(place_z);
            }

            if (world_x < 1 || world_x >= WORLD_SIZE_X - 1 ||
                world_y < 1 || world_y >= WORLD_SIZE_Y - 1 ||
                world_z < 1 || world_z >= WORLD_SIZE_Z - 1) {
                return;
            }

            world_data[world_x][world_y][world_z] = add ? 1.0f : 0.0f;
            return;
        }
    }
}