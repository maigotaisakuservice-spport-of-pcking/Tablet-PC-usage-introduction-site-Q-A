#ifndef GLOBALS_H
#define GLOBALS_H

#include <windows.h>
#include "character.h"

// --- Constants ---
#define WORLD_WIDTH 64
#define WORLD_HEIGHT 64
#define WORLD_DEPTH 64
#define MEMORY_SIZE (1024 * 1024 * 200) // 200 MB
#define MAX_DRONES 10
#define MAX_ZOMBIES 10
#define MAX_PIGS 5
#define MAX_COWS 5
#define MAX_PARTICLES 1000
#define MAX_PROJECTILES 20
#define MAX_WORLDS 5
#define INVENTORY_SLOTS 9
#define CRAFTING_GRID_SIZE 9 // 8 outer + 1 center
#define MAX_RECIPES 10


// --- Enums ---
typedef enum { STATE_TITLE, STATE_WORLD_SELECT, STATE_SETTINGS, STATE_GAME, STATE_CRAFTING } GameState;
typedef enum { MODE_NORMAL, MODE_PEACEFUL } GameMode;
typedef enum { WEATHER_CLEAR, WEATHER_RAIN, WEATHER_SNOW } WeatherType;
typedef enum { BIOME_PLAINS, BIOME_FOREST, BIOME_DESERT } BiomeType;
typedef enum {
    COLOR_BLACK, COLOR_WHITE, COLOR_RED, COLOR_GREEN, COLOR_BLUE,
    COLOR_YELLOW, COLOR_CYAN, COLOR_MAGENTA,
    COLOR_GRASS, COLOR_DIRT, COLOR_GRAY, COLOR_SAND, COLOR_WOOD,
    COLOR_LEAVES, COLOR_WATER, COLOR_SANDSTONE, COLOR_GREEN_DARK,
    COLOR_BLUE_DARK, COLOR_BROWN, COLOR_GRAY_DARK, COLOR_GRAY_LIGHT,
    COLOR_DIAMOND,
    COLOR_COUNT
} Color;

typedef enum {
    ITEM_NONE, ITEM_DIRT, ITEM_STONE, ITEM_WOOD, ITEM_IRON_PICKAXE,
    ITEM_DIAMOND, ITEM_DIAMOND_PICKAXE
} ItemType;


// --- Structs ---
typedef struct {
    unsigned char data[WORLD_WIDTH * WORLD_HEIGHT * WORLD_DEPTH];
} World;

typedef struct {
    float x, y, z;
    bool active;
    CharacterAnimation anim;
} Drone;

typedef struct {
    float x, y, z;
    bool active;
    int hp;
    CharacterAnimation anim;
} Zombie;

typedef struct {
    float x, y, z, vx, vz;
    bool active;
    int wander_timer;
    CharacterAnimation anim;
} Pig;

typedef struct {
    float x, y, z, vx, vz;
    bool active;
    int wander_timer;
    CharacterAnimation anim;
} Cow;

typedef struct {
    float x, y, z;
    bool active;
    int hp;
    int action_timer;
    int aoe_effect_timer;
} Boss;

typedef struct {
    float x, y, z, vx, vy, vz;
    bool active;
} Particle;

typedef struct {
    float x, y, z, vx, vy, vz;
    bool active;
} Projectile;

typedef struct {
    ItemType type;
    int count;
} InventorySlot;

typedef struct {
    ItemType ingredients[CRAFTING_GRID_SIZE]; // 0 is center, 1-8 are surrounding
    ItemType output;
    int output_count;
} Recipe;


// --- Global Variables (declarations) ---
extern unsigned char* worldData;
extern float camX, camY, camZ;
extern float camYaw, camPitch;
extern bool keys[256];
extern bool isSphereMode;
extern int currentColorIndex;
extern int playerHP;
extern int score;
extern Drone drones[MAX_DRONES];
extern Zombie zombies[MAX_ZOMBIES];
extern Pig pigs[MAX_PIGS];
extern Cow cows[MAX_COWS];
extern Boss boss;
extern Particle particles[MAX_PARTICLES];
extern Projectile projectiles[MAX_PROJECTILES];
extern GameState gameState;
extern GameMode gameMode;
extern WeatherType currentWeather;
extern float gameTime;
extern float sunDirection[3];
extern const float* colorPalette[COLOR_COUNT];
extern char world_names[MAX_WORLDS][32];
extern int world_count;
extern char currentWorldName[32];
extern int selectedMenuItem;
extern bool musicEnabled;
extern InventorySlot inventory[INVENTORY_SLOTS];
extern int active_inventory_slot;
extern int mouseX, mouseY;

// Crafting system globals
extern InventorySlot crafting_grid[CRAFTING_GRID_SIZE];
extern InventorySlot crafting_output;
extern InventorySlot held_item;
extern Recipe recipes[MAX_RECIPES];
extern int recipe_count;

// --- Function Prototypes ---
void draw_cube(float size); // From renderer.cpp, but used in character_renderer.cpp
bool is_in_shadow(float x, float y, float z); // From renderer.cpp
void InitializeRecipes(); // From globals.cpp

#endif // GLOBALS_H