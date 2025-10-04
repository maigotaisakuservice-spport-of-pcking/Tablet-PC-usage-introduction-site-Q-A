#include "globals.h"
#include <string.h> // For strlen in str_reverse

// World constants and data pointer
const int WORLD_WIDTH = 512;
const int WORLD_HEIGHT = 128;
const int WORLD_DEPTH = 512;
char* worldData = nullptr;

// Camera variables
float camX = 0.0f, camY = 30.0f, camZ = 5.0f;
float camYaw = 0.0f, camPitch = 0.0f;

// Build mode
bool isSphereMode = false;
int currentColorIndex = 0; // Default to 0 (Red)
const float colorPalette[9][3] = {
    {1.0f, 0.0f, 0.0f}, // 1: Red
    {0.0f, 1.0f, 0.0f}, // 2: Green
    {0.0f, 0.0f, 1.0f}, // 3: Blue
    {1.0f, 1.0f, 0.0f}, // 4: Yellow
    {1.0f, 0.0f, 1.0f}, // 5: Magenta
    {0.0f, 1.0f, 1.0f}, // 6: Cyan
    {1.0f, 1.0f, 1.0f}, // 7: White
    {0.5f, 0.5f, 0.5f}, // 8: Gray
    {0.5f, 0.2f, 0.0f}  // 9: Brown
};

// Voxel Drone
const int MAX_DRONES = 10;
Drone drones[MAX_DRONES];

// Score
int score = 0;

// Sun direction
float sunDirection[3] = {0.7f, 1.0f, 0.5f};
float gameTime = 0.0f;

// Weather System
WeatherType currentWeather = WEATHER_CLEAR;
const int MAX_PARTICLES = 1000;
Particle particles[MAX_PARTICLES];

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
    if (n == 0) {
        s[i++] = '0';
    } else {
        while (n > 0) {
            s[i++] = n % 10 + '0';
            n /= 10;
        }
    }
    if (sign < 0) s[i++] = '-';
    s[i] = '\0';
    str_reverse(s);
}