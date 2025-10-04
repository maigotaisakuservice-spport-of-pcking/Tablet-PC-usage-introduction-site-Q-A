#include "renderer.h"
#include "globals.h"
#include <GL/gl.h>
#include <math.h>

// --- Shadow System ---
bool is_in_shadow(float startX, float startY, float startZ) {
    float rayX = startX + sunDirection[0] * 0.51f;
    float rayY = startY + sunDirection[1] * 0.51f;
    float rayZ = startZ + sunDirection[2] * 0.51f;

    for (int i = 0; i < 25; ++i) {
        int blockX = floor(rayX);
        int blockY = floor(rayY);
        int blockZ = floor(rayZ);

        if (blockY >= WORLD_HEIGHT) return false;
        if (blockX < 0 || blockX >= WORLD_WIDTH || blockY < 0 || blockZ < 0 || blockZ >= WORLD_DEPTH) {
             rayX += sunDirection[0];
             rayY += sunDirection[1];
             rayZ += sunDirection[2];
             continue;
        }

        int index = blockY * (WORLD_WIDTH * WORLD_DEPTH) + blockZ * WORLD_WIDTH + blockX;
        if (worldData[index] & (1 << 7)) {
            return true;
        }

        rayX += sunDirection[0];
        rayY += sunDirection[1];
        rayZ += sunDirection[2];
    }
    return false;
}
// --- End Shadow System ---

// --- Custom Bitmap Font ---
const unsigned char FONT_DATA[17][5] = {
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, {0x00, 0x21, 0x7F, 0x01, 0x00}, {0x21, 0x43, 0x45, 0x49, 0x31},
    {0x41, 0x49, 0x49, 0x49, 0x36}, {0x18, 0x14, 0x12, 0x7F, 0x10}, {0x27, 0x45, 0x45, 0x45, 0x39},
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, {0x01, 0x71, 0x09, 0x05, 0x03}, {0x36, 0x49, 0x49, 0x49, 0x36},
    {0x06, 0x49, 0x49, 0x29, 0x1E}, {0x00, 0x00, 0x00, 0x00, 0x00}, {0x00, 0x36, 0x36, 0x00, 0x00},
    {0x4E, 0x49, 0x49, 0x49, 0x41}, {0x7F, 0x49, 0x41, 0x41, 0x63}, {0x3E, 0x41, 0x41, 0x41, 0x3E},
    {0x7F, 0x09, 0x09, 0x09, 0x06}, {0x3E, 0x41, 0x49, 0x41, 0x3E}
};

void DrawChar(float x, float y, unsigned char character_code) {
    const float pixel_size = 8.0f / 5.0f;
    glBegin(GL_QUADS);
    for (int i = 0; i < 5; i++) for (int j = 0; j < 8; j++)
        if ((FONT_DATA[character_code][i] >> j) & 1) {
            glVertex2f(x + i * pixel_size, y + j * pixel_size);
            glVertex2f(x + (i + 1) * pixel_size, y + j * pixel_size);
            glVertex2f(x + (i + 1) * pixel_size, y + (j + 1) * pixel_size);
            glVertex2f(x + i * pixel_size, y + (j + 1) * pixel_size);
        }
    glEnd();
}

void DrawText(float x, float y, const char* text) {
    for (int i = 0; text[i] != '\0'; ++i) {
        char c = text[i];
        int code = 10;
        if (c >= '0' && c <= '9') code = c - '0';
        else if (c == ':') code = 11; else if (c == 'S') code = 12;
        else if (c == 'c') code = 13; else if (c == 'o') code = 14;
        else if (c == 'r') code = 15; else if (c == 'e') code = 16;
        DrawChar(x + i * 10.0f, y, code);
    }
}
// --- End Custom Bitmap Font ---

// --- Shape Drawing ---
const float X = 0.525731112119133606f;
const float Z = 0.850650808352039932f;
const float sphere_vertices[12][3] = {
    {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z}, {0.0, Z, X}, {0.0, Z, -X},
    {0.0, -Z, X}, {0.0, -Z, -X}, {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0}
};
const int sphere_indices[20][3] = {
    {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1}, {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3},
    {2,7,3}, {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6}, {6,1,10}, {9,0,11},
    {9,11,2}, {9,2,5}, {7,2,11}
};

void DrawSphere(float x, float y, float z, const float* color, bool in_shadow) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(0.5f, 0.5f, 0.5f);
    glBegin(GL_TRIANGLES);
    float shadow_factor = in_shadow ? 0.5f : 1.0f;
    glColor3f(color[0] * shadow_factor, color[1] * shadow_factor, color[2] * shadow_factor);
    for (int i = 0; i < 20; i++) {
        glVertex3fv(&sphere_vertices[sphere_indices[i][0]][0]);
        glVertex3fv(&sphere_vertices[sphere_indices[i][1]][0]);
        glVertex3fv(&sphere_vertices[sphere_indices[i][2]][0]);
    }
    glEnd();
    glPopMatrix();
}

void DrawCube(float x, float y, float z, const float* color, bool in_shadow) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glBegin(GL_QUADS);
    float shadow_factor = in_shadow ? 0.5f : 1.0f;
    glColor3f(color[0] * shadow_factor, color[1] * shadow_factor, color[2] * shadow_factor);
    glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f); glVertex3f(-0.5f,  0.5f,  0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f, -0.5f,  0.5f); glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f); glVertex3f( 0.5f,  0.5f, -0.5f); glVertex3f( 0.5f,  0.5f,  0.5f); glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f,  0.5f); glVertex3f(-0.5f,  0.5f,  0.5f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glEnd();
    glPopMatrix();
}
// --- End Shape Drawing ---

static HDC hDC;
static HGLRC hRC;

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

void Render() {
    float sun_norm = (sunDirection[1] - 0.4f) / 0.8f;
    if (sun_norm < 0.0f) sun_norm = 0.0f;
    if (sun_norm > 1.0f) sun_norm = 1.0f;
    float r = 0.8f * (1.0f - sun_norm) + 0.3f * sun_norm;
    float g = 0.4f * (1.0f - sun_norm) + 0.6f * sun_norm;
    float b = 0.4f * (1.0f - sun_norm) + 0.9f * sun_norm;
    glClearColor(r, g, b, 1.0f);

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
    for (int x = floor(camX) - RENDER_DISTANCE; x < floor(camX) + RENDER_DISTANCE; ++x) {
        for (int y = 0; y < WORLD_HEIGHT; ++y) {
            for (int z = floor(camZ) - RENDER_DISTANCE; z < floor(camZ) + RENDER_DISTANCE; ++z) {
                 if (x >= 0 && x < WORLD_WIDTH && y >= 0 && y < WORLD_HEIGHT && z >= 0 && z < WORLD_DEPTH) {
                    int index = y * (WORLD_WIDTH * WORLD_DEPTH) + z * WORLD_WIDTH + x;
                    unsigned char blockData = worldData[index];
                    if (blockData & (1 << 7)) {
                        bool shadow = is_in_shadow((float)x, (float)y, (float)z);
                        bool isSphereBlock = blockData & (1 << 4);
                        int colorIndex = blockData & 0x0F;
                        const float* color = colorPalette[colorIndex];
                        if (isSphereBlock) DrawSphere((float)x, (float)y, (float)z, color, shadow);
                        else DrawCube((float)x, (float)y, (float)z, color, shadow);
                    }
                }
            }
        }
    }

    const float droneColor[] = {0.2f, 0.2f, 0.2f};
    for (int i = 0; i < MAX_DRONES; ++i)
        if (drones[i].active) DrawCube(drones[i].x, drones[i].y, drones[i].z, droneColor, false);

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

    char scoreText[12];
    tiny_itoa(score, scoreText);
    DrawText(10, 10, "Score:");
    DrawText(70, 10, scoreText);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    SwapBuffers(hDC);
}