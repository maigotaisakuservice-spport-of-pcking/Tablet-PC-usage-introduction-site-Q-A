#ifndef GLOBALS_H
#define GLOBALS_H

#include <windows.h>
#include "character.h"

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
    CharacterAnimation anim;
};
extern const int MAX_DRONES;
extern Drone drones[10];

// Zombie
struct Zombie {
    float x, y, z;
    bool active;
    int hp;
    CharacterAnimation anim;
};
extern const int MAX_ZOMBIES;
extern Zombie zombies[];

// Passive Mobs
struct Pig {
    float x, y, z;
    bool active;
    CharacterAnimation anim;
    float vx, vz;
    int wander_timer;
};
extern const int MAX_PIGS;
extern Pig pigs[];

struct Cow {
    float x, y, z;
    bool active;
    CharacterAnimation anim;
    float vx, vz;
    int wander_timer;
};
extern const int MAX_COWS;
extern Cow cows[];

// Score
extern int score;

// Player
extern int playerHP;

// --- Ultra-lightweight Integer to String ---
void str_reverse(char *str);
void tiny_itoa(int n, char* s);

// Sun direction
extern float sunDirection[3];
extern float gameTime;

// Game State
enum GameState {
    STATE_TITLE,
    STATE_GAME,
    STATE_SETTINGS
};
extern GameState gameState;
extern int selectedMenuItem;

enum GameMode {
    MODE_NORMAL,
    MODE_PEACEFUL
};
extern GameMode gameMode;
extern bool musicEnabled;

// Boss System
struct VoxelOverlord {
    bool active;
    float x, y, z;
    int hp;
    int action_timer;
    int aoe_effect_timer;
};
extern VoxelOverlord boss;

// Projectile System
struct Projectile {
    bool active;
    float x, y, z;
    float vx, vy, vz;
};
extern const int MAX_PROJECTILES;
extern Projectile projectiles[];

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