#include <windows.h>
#include <GL/gl.h>
#include <math.h>

// --- Sound ---
DWORD WINAPI PlaySoundThread(LPVOID lpParam) {
    int* params = (int*)lpParam;
    Beep(params[0], params[1]);
    delete[] params;
    return 0;
}

void PlaySoundEffect(int frequency, int duration) {
    int* params = new int[2];
    params[0] = frequency;
    params[1] = duration;
    CreateThread(NULL, 0, PlaySoundThread, params, 0, NULL);
}
// --- End of Sound ---

// --- Lightweight Noise Implementation ---
// Simple hashing function for noise generation
unsigned int simple_hash(unsigned int x, unsigned int y) {
    unsigned int h = x * 374761393 + y * 668265263; // Prime numbers
    h = (h ^ (h >> 13)) * 1274126177;
    return h ^ (h >> 16);
}

// Simple 2D Value Noise
float value_noise(float x, float y) {
    int ix = floor(x);
    int iy = floor(y);
    float fx = x - ix;
    float fy = y - iy;

    unsigned int p00 = simple_hash(ix, iy);
    unsigned int p10 = simple_hash(ix + 1, iy);
    unsigned int p01 = simple_hash(ix, iy + 1);
    unsigned int p11 = simple_hash(ix + 1, iy + 1);

    float v00 = (p00 & 0xFFFF) / 65535.0f;
    float v10 = (p10 & 0xFFFF) / 65535.0f;
    float v01 = (p01 & 0xFFFF) / 65535.0f;
    float v11 = (p11 & 0xFFFF) / 65535.0f;

    // Bilinear interpolation
    float ix1 = v00 * (1.0f - fx) + v10 * fx;
    float ix2 = v01 * (1.0f - fx) + v11 * fx;
    return ix1 * (1.0f - fy) + ix2 * fy;
}

// Octave noise using the simple value noise
float octave_noise(float x, float y, int octaves, float persistence) {
    float total = 0;
    float frequency = 1;
    float amplitude = 1;
    float maxValue = 0;
    for(int i=0; i<octaves; i++) {
        total += value_noise(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2;
    }
    return total/maxValue;
}
// --- End of Noise Implementation ---

// World constants and data pointer
const int WORLD_WIDTH = 512;
const int WORLD_HEIGHT = 128;
const int WORLD_DEPTH = 512;
char* worldData = nullptr;

// Camera variables
float camX = 0.0f, camY = 30.0f, camZ = 5.0f;
float camYaw = 0.0f, camPitch = 0.0f;

// Voxel Drone
struct Drone {
    float x, y, z;
    bool active;
};
const int MAX_DRONES = 10;
Drone drones[MAX_DRONES];

int score = 0;

HDC hDC;
HGLRC hRC;

// --- Ultra-lightweight Integer to String ---
void str_reverse(char *str) {
    char *p1, *p2;
    if (!str || !*str) return;
    for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }
}

void tiny_itoa(int n, char* s) {
    int i = 0;
    int sign = n;
    if (sign < 0) n = -n;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0) s[i++] = '-';
    s[i] = '\0';
    str_reverse(s);
}
// --- End Integer to String ---

// --- Custom Bitmap Font ---
const unsigned char FONT_DATA[17][5] = {
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 0
    {0x00, 0x21, 0x7F, 0x01, 0x00}, // 1
    {0x21, 0x43, 0x45, 0x49, 0x31}, // 2
    {0x41, 0x49, 0x49, 0x49, 0x36}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x4E, 0x49, 0x49, 0x49, 0x41}, // S
    {0x7F, 0x49, 0x41, 0x41, 0x63}, // c
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // o
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // r
    {0x3E, 0x41, 0x49, 0x41, 0x3E}  // e
};

void DrawChar(float x, float y, unsigned char character_code) {
    const float char_width = 8.0f;
    const float char_height = 10.0f;
    const float pixel_size = char_width / 5.0f;

    glBegin(GL_QUADS);
    for (int i = 0; i < 5; i++) { // 5 columns
        for (int j = 0; j < 8; j++) { // 8 rows
            if ((FONT_DATA[character_code][i] >> j) & 1) {
                glVertex2f(x + i * pixel_size, y + j * pixel_size);
                glVertex2f(x + (i + 1) * pixel_size, y + j * pixel_size);
                glVertex2f(x + (i + 1) * pixel_size, y + (j + 1) * pixel_size);
                glVertex2f(x + i * pixel_size, y + (j + 1) * pixel_size);
            }
        }
    }
    glEnd();
}

void DrawText(float x, float y, const char* text) {
    for (int i = 0; text[i] != '\0'; ++i) {
        char c = text[i];
        int code = 10; // Default to space
        if (c >= '0' && c <= '9') code = c - '0';
        else if (c == ':') code = 11;
        else if (c == 'S') code = 12;
        else if (c == 'c') code = 13;
        else if (c == 'o') code = 14;
        else if (c == 'r') code = 15;
        else if (c == 'e') code = 16;

        DrawChar(x + i * 10.0f, y, code);
    }
}
// --- End Custom Bitmap Font ---

void EnableOpenGL(HWND hwnd) {
    PIXELFORMATDESCRIPTOR pfd;
    int format;
    hDC = GetDC(hwnd);
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    format = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, format, &pfd);
    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);
    glEnable(GL_DEPTH_TEST);
}

void DisableOpenGL(HWND hwnd) {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

void DrawCube(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glBegin(GL_QUADS);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f);
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f);
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f(-0.5f,  0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f( 0.5f,  0.5f, -0.5f);
        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f(-0.5f, -0.5f,  0.5f);
        glColor3f(1.0f, 0.0f, 1.0f);
        glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f( 0.5f, -0.5f,  0.5f);
        glColor3f(0.0f, 1.0f, 1.0f);
        glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glEnd();
    glPopMatrix();
}

void Render() {
    glClearColor(0.3f, 0.6f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = 800.0f / 600.0f;
    float fovY = 45.0f;
    float zNear = 0.1f;
    float zFar = 500.0f;
    float top = tan(fovY * 3.14159265 / 360.0) * zNear;
    float bottom = -top;
    float right = top * aspect;
    float left = -right;
    glFrustum(left, right, bottom, top, zNear, zFar);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(camPitch, 1.0f, 0.0f, 0.0f);
    glRotatef(camYaw, 0.0f, 1.0f, 0.0f);
    glTranslatef(-camX, -camY, -camZ);

    const int RENDER_DISTANCE = 16;
    int playerX = (int)floor(camX);
    int playerY = (int)floor(camY);
    int playerZ = (int)floor(camZ);
    for (int x = playerX - RENDER_DISTANCE; x < playerX + RENDER_DISTANCE; ++x) {
        for (int y = 0; y < WORLD_HEIGHT; ++y) {
            for (int z = playerZ - RENDER_DISTANCE; z < playerZ + RENDER_DISTANCE; ++z) {
                if (x >= 0 && x < WORLD_WIDTH && y >= 0 && y < WORLD_HEIGHT && z >= 0 && z < WORLD_DEPTH) {
                    int index = y * (WORLD_WIDTH * WORLD_DEPTH) + z * WORLD_WIDTH + x;
                    if (worldData[index] == 1) {
                        DrawCube((float)x, (float)y, (float)z);
                    }
                }
            }
        }
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 800, 600, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2i(395, 300); glVertex2i(405, 300);
    glVertex2i(400, 295); glVertex2i(400, 305);
    glEnd();
    // --- Draw Drones ---
    glColor3f(0.2f, 0.2f, 0.2f); // Dark grey for drones
    for (int i = 0; i < MAX_DRONES; ++i) {
        if (drones[i].active) {
            DrawCube(drones[i].x, drones[i].y, drones[i].z);
        }
    }

    // --- Draw 2D UI ---
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 800, 600, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Crosshair
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2i(395, 300); glVertex2i(405, 300);
    glVertex2i(400, 295); glVertex2i(400, 305);
    glEnd();

    // Score
    DrawText(10, 10, "Score:");
    char scoreNumText[12];
    tiny_itoa(score, scoreNumText);
    DrawText(70, 10, scoreNumText); // Draw number after "Score:"

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    SwapBuffers(hDC);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            EnableOpenGL(hwnd);
            SetCursorPos(400, 300);
            break;
        case WM_CLOSE:
            DisableOpenGL(hwnd);
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_KEYDOWN:
            {
                float speed = 0.2f;
                float yawRad = camYaw * 3.14159265f / 180.0f;

                // Standard FPS movement logic
                float forwardX = sin(yawRad);
                float forwardZ = -cos(yawRad);

                switch(wParam) {
                    case 'W': camX += forwardX * speed; camZ += forwardZ * speed; break; // Forward
                    case 'S': camX -= forwardX * speed; camZ -= forwardZ * speed; break; // Backward
                    case 'A': camX -= forwardZ * speed; camZ += forwardX * speed; break; // Strafe Left
                    case 'D': camX += forwardZ * speed; camZ -= forwardX * speed; break; // Strafe Right
                    case VK_ESCAPE: PostQuitMessage(0); break;
                }
            }
            break;
        case WM_MOUSEMOVE: {
            int xPos = LOWORD(lParam);
            int yPos = HIWORD(lParam);
            int deltaX = xPos - 400;
            int deltaY = yPos - 300;
            camYaw += deltaX * 0.1f;
            camPitch -= deltaY * 0.1f;
            if(camPitch > 89.0f) camPitch = 89.0f;
            if(camPitch < -89.0f) camPitch = -89.0f;
            SetCursorPos(400, 300);
            } break;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN: {
            float rayX = camX, rayY = camY, rayZ = camZ;
            float pitchRad = camPitch * 3.14159 / 180.0;
            float yawRad = camYaw * 3.14159 / 180.0;
            float dirX = sin(yawRad) * cos(pitchRad);
            float dirY = -sin(pitchRad);
            float dirZ = -cos(yawRad) * cos(pitchRad);
            int lastX=0, lastY=0, lastZ=0;
            for (float step = 0; step < 8.0f; step += 0.05f) {
                lastX = (int)floor(rayX); lastY = (int)floor(rayY); lastZ = (int)floor(rayZ);
                rayX += dirX * 0.05f; rayY += dirY * 0.05f; rayZ += dirZ * 0.05f;

                if (msg == WM_LBUTTONDOWN) {
                    bool droneHit = false;
                    for (int i = 0; i < MAX_DRONES; ++i) {
                        if (drones[i].active) {
                            float dx = drones[i].x - rayX;
                            float dy = drones[i].y - rayY;
                            float dz = drones[i].z - rayZ;
                            if ((dx * dx + dy * dy + dz * dz) < 1.0f) { // Drone hitbox
                                drones[i].active = false;
                                score++;
                                PlaySoundEffect(150, 150); // Drone destruction sound
                                droneHit = true;
                                break;
                            }
                        }
                    }
                    if (droneHit) break;
                }

                int blockX = (int)floor(rayX), blockY = (int)floor(rayY), blockZ = (int)floor(rayZ);
                if (blockX < 0 || blockX >= WORLD_WIDTH || blockY < 0 || blockY >= WORLD_HEIGHT || blockZ < 0 || blockZ >= WORLD_DEPTH) break;
                int index = blockY * (WORLD_WIDTH * WORLD_DEPTH) + blockZ * WORLD_WIDTH + blockX;
                if (worldData[index] == 1) {
                    if (msg == WM_LBUTTONDOWN) {
                        worldData[index] = 0;
                        PlaySoundEffect(300, 50); // Block destruction sound
                    } else if (msg == WM_RBUTTONDOWN) {
                        int prevIndex = lastY * (WORLD_WIDTH * WORLD_DEPTH) + lastZ * WORLD_WIDTH + lastX;
                        if (lastX >= 0 && lastX < WORLD_WIDTH && lastY >= 0 && lastY < WORLD_HEIGHT && lastZ >= 0 && lastZ < WORLD_DEPTH) {
                            worldData[prevIndex] = 1;
                            PlaySoundEffect(600, 50); // Block placement sound
                        }
                    } else { // WM_MBUTTONDOWN
                        const int blast_radius = 4;
                        for (int dx = -blast_radius; dx <= blast_radius; ++dx)
                        for (int dy = -blast_radius; dy <= blast_radius; ++dy)
                        for (int dz = -blast_radius; dz <= blast_radius; ++dz)
                            if (dx*dx + dy*dy + dz*dz <= blast_radius*blast_radius) {
                                int effectX = blockX + dx, effectY = blockY + dy, effectZ = blockZ + dz;
                                if (effectX >= 0 && effectX < WORLD_WIDTH && effectY >= 0 && effectY < WORLD_HEIGHT && effectZ >= 0 && effectZ < WORLD_DEPTH) {
                                            if (worldData[effectY * (WORLD_WIDTH * WORLD_DEPTH) + effectZ * WORLD_WIDTH + effectX] == 1) {
                                                worldData[effectY * (WORLD_WIDTH * WORLD_DEPTH) + effectZ * WORLD_WIDTH + effectX] = 0;
                                            }
                                }
                            }
                            PlaySoundEffect(100, 200); // Deconstructor explosion sound
                    }
                    break;
                }
            }
            } break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;
    BOOL bQuit = FALSE;

    wc.cbSize = sizeof(WNDCLASSEX); wc.style = 0; wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0; wc.cbWndExtra = 0; wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); wc.lpszMenuName = NULL;
    wc.lpszClassName = "SmallCraft50KWindowClass"; wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) { MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK); return 0; }

    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "SmallCraft50KWindowClass", "SmallCraft 50K", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) { MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK); return 0; }

    const SIZE_T MEMORY_SIZE = 200 * 1024 * 1024;
    LPVOID memoryBlock = VirtualAlloc(NULL, MEMORY_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (memoryBlock == NULL) { MessageBox(NULL, "Memory Allocation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK); return 0; }

    if ((long long)WORLD_WIDTH * WORLD_HEIGHT * WORLD_DEPTH > MEMORY_SIZE) {
         MessageBox(NULL, "World size exceeds allocated memory!", "Error!", MB_ICONEXCLAMATION | MB_OK);
         VirtualFree(memoryBlock, 0, MEM_RELEASE); return 0;
    }
    worldData = (char*)memoryBlock;
    for (int x = 0; x < WORLD_WIDTH; ++x) {
        for (int z = 0; z < WORLD_DEPTH; ++z) {
            const float noise = octave_noise(x * 0.015f, z * 0.015f, 5, 0.5f);
            const int terrainHeight = static_cast<int>(noise * (WORLD_HEIGHT * 0.75));
            for (int y = 0; y < WORLD_HEIGHT; ++y) {
                worldData[y * (WORLD_WIDTH * WORLD_DEPTH) + z * WORLD_WIDTH + x] = (y < terrainHeight) ? 1 : 0;
            }
        }
    }

    for(int i=0; i<MAX_DRONES; ++i) drones[i].active = false;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (!bQuit) {
        if (rand() % 1000 < 5) { // Spawn a drone
            for (int i = 0; i < MAX_DRONES; ++i) {
                if (!drones[i].active) {
                    drones[i].active = true;
                    drones[i].x = camX + (rand() % 20 - 10);
                    drones[i].y = camY + (rand() % 5);
                    drones[i].z = camZ + (rand() % 20 - 10);
                    break;
                }
            }
        }

        if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) {
            if (Msg.message == WM_QUIT) bQuit = TRUE;
            else { TranslateMessage(&Msg); DispatchMessage(&Msg); }
        } else {
            // Update drone positions
            for(int i=0; i<MAX_DRONES; ++i) {
                if(drones[i].active) {
                    float dx = camX - drones[i].x;
                    float dy = camY - drones[i].y;
                    float dz = camZ - drones[i].z;
                    float dist = sqrt(dx*dx + dy*dy + dz*dz);
                    if (dist > 0) {
                        drones[i].x += (dx / dist) * 0.05f; // Drone speed
                        drones[i].y += (dy / dist) * 0.05f;
                        drones[i].z += (dz / dist) * 0.05f;
                    }
                }
            }
            Render();
        }
    }

    VirtualFree(memoryBlock, 0, MEM_RELEASE);
    return Msg.wParam;
}