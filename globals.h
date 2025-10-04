#ifndef GLOBALS_H
#define GLOBALS_H

#include <windows.h>

// World constants and data pointer
extern const int WORLD_WIDTH;
extern const int WORLD_HEIGHT;
extern const int WORLD_DEPTH;
extern char* worldData;

// Camera variables
extern float camX, camY, camZ;
extern float camYaw, camPitch;

// Build mode
extern bool isSphereMode;
extern int currentColorIndex;
extern const float colorPalette[9][3];

// Voxel Drone
struct Drone {
    float x, y, z;
    bool active;
};
extern const int MAX_DRONES;
extern Drone drones[10];

// Score
extern int score;

// --- Ultra-lightweight Integer to String ---
void str_reverse(char *str);
void tiny_itoa(int n, char* s);

// Sun direction
extern float sunDirection[3];
extern float gameTime;

// Weather System
enum WeatherType {
    WEATHER_CLEAR,
    WEATHER_RAIN,
    WEATHER_SNOW
};
struct Particle {
    float x, y, z;
    float vx, vy, vz;
    bool active;
};
extern WeatherType currentWeather;
extern const int MAX_PARTICLES;
extern Particle particles[];


#endif // GLOBALS_H