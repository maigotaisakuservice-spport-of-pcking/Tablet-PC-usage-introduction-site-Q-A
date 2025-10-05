#include "globals.h"

// --- Global Variables (definitions) ---
unsigned char* worldData = NULL;
float camX = WORLD_WIDTH / 2.0f;
float camY = WORLD_HEIGHT;
float camZ = WORLD_DEPTH / 2.0f;
float camYaw = 0.0f;
float camPitch = 0.0f;
bool keys[256] = {false};
bool isSphereMode = false;
int currentColorIndex = 0;
int playerHP = 20;
int score = 0;
Drone drones[MAX_DRONES];
Zombie zombies[MAX_ZOMBIES];
Pig pigs[MAX_PIGS];
Cow cows[MAX_COWS];
Boss boss;
Particle particles[MAX_PARTICLES];
Projectile projectiles[MAX_PROJECTILES];
GameState gameState = STATE_TITLE;
GameMode gameMode = MODE_NORMAL;
WeatherType currentWeather = WEATHER_CLEAR;
float gameTime = 0.75f; // Start in the morning
float sunDirection[3] = {0.0f, 1.0f, 0.0f};
char world_names[MAX_WORLDS][32];
int world_count = 0;
char currentWorldName[32];
int selectedMenuItem = 0;
bool musicEnabled = true;
InventorySlot inventory[INVENTORY_SLOTS];
int active_inventory_slot = 0;
int mouseX = 0;
int mouseY = 0;


// --- Color Palette Definition ---
const float color_black[] = {0.0f, 0.0f, 0.0f};
const float color_white[] = {1.0f, 1.0f, 1.0f};
const float color_red[] = {1.0f, 0.0f, 0.0f};
const float color_green[] = {0.0f, 1.0f, 0.0f};
const float color_blue[] = {0.0f, 0.0f, 1.0f};
const float color_yellow[] = {1.0f, 1.0f, 0.0f};
const float color_cyan[] = {0.0f, 1.0f, 1.0f};
const float color_magenta[] = {1.0f, 0.0f, 1.0f};
const float color_grass[] = {0.1f, 0.8f, 0.2f};
const float color_dirt[] = {0.6f, 0.4f, 0.2f};
const float color_gray[] = {0.5f, 0.5f, 0.5f};
const float color_sand[] = {0.9f, 0.9f, 0.7f};
const float color_wood[] = {0.4f, 0.3f, 0.1f};
const float color_leaves[] = {0.1f, 0.6f, 0.1f};
const float color_water[] = {0.2f, 0.4f, 0.9f};
const float color_sandstone[] = {0.8f, 0.8f, 0.6f};
const float color_green_dark[] = {0.0f, 0.4f, 0.0f};
const float color_blue_dark[] = {0.0f, 0.0f, 0.5f};
const float color_brown[] = {0.35f, 0.25f, 0.1f};
const float color_gray_dark[] = {0.3f, 0.3f, 0.3f};
const float color_gray_light[] = {0.8f, 0.8f, 0.8f};
const float color_diamond[] = {0.6f, 0.9f, 1.0f};

const float* colorPalette[COLOR_COUNT] = {
    color_black, color_white, color_red, color_green, color_blue,
    color_yellow, color_cyan, color_magenta,
    color_grass, color_dirt, color_gray, color_sand, color_wood,
    color_leaves, color_water, color_sandstone, color_green_dark,
    color_blue_dark, color_brown, color_gray_dark, color_gray_light,
    color_diamond
};

// --- Crafting System Globals (definitions) ---
InventorySlot crafting_grid[CRAFTING_GRID_SIZE];
InventorySlot crafting_output;
InventorySlot held_item = { ITEM_NONE, 0 };
Recipe recipes[MAX_RECIPES];
int recipe_count = 0;

void InitializeRecipes() {
    // Recipe 1: Diamond Pickaxe
    // Requires: 1 Iron Pickaxe (center), 3 Diamonds (surrounding)
    recipes[0].ingredients[0] = ITEM_IRON_PICKAXE;
    recipes[0].ingredients[1] = ITEM_DIAMOND;
    recipes[0].ingredients[2] = ITEM_DIAMOND;
    recipes[0].ingredients[3] = ITEM_DIAMOND;
    for(int i = 4; i < CRAFTING_GRID_SIZE; ++i) recipes[0].ingredients[i] = ITEM_NONE;
    recipes[0].output = ITEM_DIAMOND_PICKAXE;
    recipes[0].output_count = 1;

    // Add more recipes here...

    recipe_count = 1; // We have one recipe so far
}