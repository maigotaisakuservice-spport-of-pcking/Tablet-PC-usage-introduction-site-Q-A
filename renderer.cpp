#include "globals.h"
#include "renderer.h"
#include "character_renderer.h"
#include "crafting.h"
#include "utils.h"
#include <GL/gl.h>
#include <math.h>
#include <string.h>

// --- Static variables for rendering ---
static HDC hDC;
static HGLRC hRC;

// --- Shape Drawing Primitives ---
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

// --- UI & Text Rendering ---
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

void RenderHotbar() {
    float slot_size = 40;
    float padding = 5;
    float start_x = (800 - (INVENTORY_SLOTS * (slot_size + padding))) / 2.0f;
    float start_y = 550;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity();
    glOrtho(0, 800, 600, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < INVENTORY_SLOTS; ++i) {
        float x = start_x + i * (slot_size + padding);
        if (i == active_inventory_slot) glColor4f(0.8f, 0.8f, 0.8f, 0.6f);
        else glColor4f(0.2f, 0.2f, 0.2f, 0.6f);

        glBegin(GL_QUADS);
        glVertex2f(x, start_y); glVertex2f(x + slot_size, start_y);
        glVertex2f(x + slot_size, start_y + slot_size); glVertex2f(x, start_y + slot_size);
        glEnd();

        if (inventory[i].type != ITEM_NONE) {
            const float* color = colorPalette[COLOR_GRAY];
            if(inventory[i].type == ITEM_DIRT) color = colorPalette[COLOR_DIRT];
            else if(inventory[i].type == ITEM_STONE) color = colorPalette[COLOR_GRAY];
            else if(inventory[i].type == ITEM_WOOD) color = colorPalette[COLOR_WOOD];
            else if(inventory[i].type == ITEM_IRON_PICKAXE) color = colorPalette[COLOR_CYAN];

            glColor3fv(color);
            float item_inset = 5;
            glBegin(GL_QUADS);
            glVertex2f(x + item_inset, start_y + item_inset); glVertex2f(x + slot_size - item_inset, start_y + item_inset);
            glVertex2f(x + slot_size - item_inset, start_y + slot_size - item_inset); glVertex2f(x + item_inset, start_y + slot_size - item_inset);
            glEnd();

            char count_text[4];
            tiny_itoa(inventory[i].count, count_text);
            glColor3f(1.0f, 1.0f, 1.0f);
            DrawText(x + slot_size - 15, start_y + slot_size - 15, count_text);
        }
    }
    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// --- Shadow Calculation ---
bool is_in_shadow(float x, float y, float z) {
    float rayX = x + 0.5f + sunDirection[0] * 0.1f;
    float rayY = y + 0.5f + sunDirection[1] * 0.1f;
    float rayZ = z + 0.5f + sunDirection[2] * 0.1f;

    for (float step = 0; step < 15.0f; step += 0.5f) {
        int blockX = floor(rayX);
        int blockY = floor(rayY);
        int blockZ = floor(rayZ);

        if (blockX < 0 || blockX >= WORLD_WIDTH || blockY < 0 || blockY >= WORLD_HEIGHT || blockZ < 0 || blockZ >= WORLD_DEPTH) {
            return false; // Ray reached out of bounds, not in shadow
        }

        int index = blockY * (WORLD_WIDTH * WORLD_DEPTH) + blockZ * WORLD_WIDTH + blockX;
        if (worldData[index] & (1 << 7)) {
            return true; // Ray hit a block, so it's in shadow
        }

        rayX += sunDirection[0] * 0.5f;
        rayY += sunDirection[1] * 0.5f;
        rayZ += sunDirection[2] * 0.5f;
    }
    return false; // Ray didn't hit anything, not in shadow
}


// --- Generic cube for characters ---
void draw_cube(float size) {
    float s = size * 0.5f;
    glBegin(GL_QUADS);
    // Front Face
    glVertex3f(-s, -s, s); glVertex3f(s, -s, s); glVertex3f(s, s, s); glVertex3f(-s, s, s);
    // Back Face
    glVertex3f(-s, -s, -s); glVertex3f(-s, s, -s); glVertex3f(s, s, -s); glVertex3f(s, -s, -s);
    // Top Face
    glVertex3f(-s, s, -s); glVertex3f(-s, s, s); glVertex3f(s, s, s); glVertex3f(s, s, -s);
    // Bottom Face
    glVertex3f(-s, -s, -s); glVertex3f(s, -s, -s); glVertex3f(s, -s, s); glVertex3f(-s, -s, s);
    // Right face
    glVertex3f(s, -s, -s); glVertex3f(s, s, -s); glVertex3f(s, s, s); glVertex3f(s, -s, s);
    // Left Face
    glVertex3f(-s, -s, -s); glVertex3f(-s, -s, s); glVertex3f(-s, s, s); glVertex3f(-s, s, -s);
    glEnd();
}


// --- Main Render Functions ---
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

    for (int i = 0; i < MAX_DRONES; ++i) {
        if (drones[i].active) {
            float dx = camX - drones[i].x;
            float dz = camZ - drones[i].z;
            float rot_y = atan2(dx, dz) * 180.0f / 3.14159265f + 180.0f;
            render_advanced_drone(drones[i].x, drones[i].y, drones[i].z, rot_y, drones[i].anim);
        }
    }
    for (int i = 0; i < MAX_ZOMBIES; ++i) {
        if (zombies[i].active) {
            float dx = camX - zombies[i].x;
            float dz = camZ - zombies[i].z;
            float rot_y = atan2(dx, dz) * 180.0f / 3.14159265f + 180.0f;
            render_voxel_zombie(zombies[i].x, zombies[i].y, zombies[i].z, rot_y, zombies[i].anim);
        }
    }

    for (int i = 0; i < MAX_PIGS; ++i) {
        if (pigs[i].active) {
            float rot_y = atan2(pigs[i].vx, pigs[i].vz) * 180.0f / 3.14159265f;
            render_pig(pigs[i].x, pigs[i].y, pigs[i].z, rot_y, pigs[i].anim);
        }
    }
    for (int i = 0; i < MAX_COWS; ++i) {
        if (cows[i].active) {
            float rot_y = atan2(cows[i].vx, cows[i].vz) * 180.0f / 3.14159265f;
            render_cow(cows[i].x, cows[i].y, cows[i].z, rot_y, cows[i].anim);
        }
    }

    // TODO: Implement proper rendering for the boss
    // DrawBoss();

    if (boss.active && boss.aoe_effect_timer > 0) {
        glPushMatrix();
        glTranslatef(boss.x, boss.y, boss.z);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        const float aoe_radius = 10.0f;
        float effect_progress = 1.0f - (boss.aoe_effect_timer / 50.0f);
        glColor4f(0.8f, 0.2f, 1.0f, 0.5f * (1.0f - effect_progress));
        glScalef(aoe_radius * effect_progress, aoe_radius * effect_progress, aoe_radius * effect_progress);
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < 20; i++) {
            glVertex3fv(&sphere_vertices[sphere_indices[i][0]][0]);
            glVertex3fv(&sphere_vertices[sphere_indices[i][1]][0]);
            glVertex3fv(&sphere_vertices[sphere_indices[i][2]][0]);
        }
        glEnd();
        glDisable(GL_BLEND);
        glPopMatrix();
    }

    const float projectileColor[] = {1.0f, 0.5f, 0.0f};
    for (int i = 0; i < MAX_PROJECTILES; ++i) {
        if (projectiles[i].active) {
            glPushMatrix();
            glTranslatef(projectiles[i].x, projectiles[i].y, projectiles[i].z);
            glScalef(0.2f, 0.2f, 0.2f);
            DrawCube(0, 0, 0, projectileColor, false);
            glPopMatrix();
        }
    }

    if (currentWeather != WEATHER_CLEAR) {
        glPointSize(currentWeather == WEATHER_RAIN ? 2.0f : 3.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_POINTS);
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].active) {
                if (currentWeather == WEATHER_RAIN) glColor3f(0.6f, 0.7f, 1.0f);
                else glColor3f(1.0f, 1.0f, 1.0f);
                glVertex3f(particles[i].x, particles[i].y, particles[i].z);
            }
        }
        glEnd();
        glDisable(GL_BLEND);
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

    char scoreText[12];
    tiny_itoa(score, scoreText);
    DrawText(10, 10, "Score:");
    DrawText(70, 10, scoreText);

    char hpText[12];
    tiny_itoa(playerHP, hpText);
    DrawText(10, 25, "HP:");
    DrawText(40, 25, hpText);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    RenderHotbar();

    SwapBuffers(hDC);
}

void RenderTitleScreen() {
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 800, 600, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);
    DrawText(280, 100, "SmallCraft 50K");

    glColor3f(selectedMenuItem == 0 ? 1.0f : 0.7f, selectedMenuItem == 0 ? 1.0f : 0.7f, selectedMenuItem == 0 ? 0.0f : 0.7f);
    DrawText(350, 250, "Survival");

    glColor3f(selectedMenuItem == 1 ? 1.0f : 0.7f, selectedMenuItem == 1 ? 1.0f : 0.7f, selectedMenuItem == 1 ? 0.0f : 0.7f);
    DrawText(350, 300, "Creative");

    glColor3f(selectedMenuItem == 2 ? 1.0f : 0.7f, selectedMenuItem == 2 ? 1.0f : 0.7f, selectedMenuItem == 2 ? 0.0f : 0.7f);
    DrawText(350, 350, "Settings");

    glColor3f(selectedMenuItem == 3 ? 1.0f : 0.7f, selectedMenuItem == 3 ? 1.0f : 0.7f, selectedMenuItem == 3 ? 0.0f : 0.7f);
    DrawText(350, 400, "Exit");

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    SwapBuffers(hDC);
}

void RenderWorldSelectScreen() {
    glClearColor(0.1f, 0.2f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 800, 600, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);
    DrawText(320, 100, "Select World");

    for (int i = 0; i < world_count; ++i) {
        glColor3f(selectedMenuItem == i ? 1.0f : 0.7f, selectedMenuItem == i ? 1.0f : 0.7f, selectedMenuItem == i ? 0.0f : 0.7f);
        DrawText(300, 200 + i * 40, world_names[i]);
    }

    if (world_count < 5) {
        glColor3f(selectedMenuItem == world_count ? 1.0f : 0.7f, selectedMenuItem == world_count ? 1.0f : 0.7f, selectedMenuItem == world_count ? 0.0f : 0.7f);
        DrawText(300, 200 + world_count * 40, "Create New World");
    }

    glColor3f(selectedMenuItem == 5 ? 1.0f : 0.7f, selectedMenuItem == 5 ? 1.0f : 0.7f, selectedMenuItem == 5 ? 0.0f : 0.7f);
    DrawText(360, 450, "Back");

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    SwapBuffers(hDC);
}

void RenderSettingsScreen() {
    glClearColor(0.2f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 800, 600, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);
    DrawText(350, 100, "Settings");

    glColor3f(selectedMenuItem == 0 ? 1.0f : 0.7f, selectedMenuItem == 0 ? 1.0f : 0.7f, selectedMenuItem == 0 ? 0.0f : 0.7f);
    char musicStatus[16];
    tiny_strcpy(musicStatus, "Music: ");
    if (musicEnabled) tiny_strcpy(musicStatus + 7, "ON");
    else tiny_strcpy(musicStatus + 7, "OFF");
    DrawText(320, 250, musicStatus);

    glColor3f(selectedMenuItem == 1 ? 1.0f : 0.7f, selectedMenuItem == 1 ? 1.0f : 0.7f, selectedMenuItem == 1 ? 0.0f : 0.7f);
    DrawText(360, 400, "Back");

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    SwapBuffers(hDC);
}

// Helper function to draw a single UI item slot
void DrawItemSlot(float x, float y, float size, const InventorySlot& slot, bool is_center_slot) {
    // Draw slot background
    if (is_center_slot) {
        glColor4f(0.6f, 0.5f, 0.2f, 0.5f); // Special color for the central "altar" slot
    } else {
        glColor4f(0.2f, 0.2f, 0.2f, 0.5f);
    }
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + size, y);
    glVertex2f(x + size, y + size);
    glVertex2f(x, y + size);
    glEnd();

    // Draw item in slot
    if (slot.type != ITEM_NONE) {
        const float* color = colorPalette[COLOR_GRAY];
        if (slot.type == ITEM_DIRT) color = colorPalette[COLOR_DIRT];
        else if (slot.type == ITEM_STONE) color = colorPalette[COLOR_GRAY];
        else if (slot.type == ITEM_WOOD) color = colorPalette[COLOR_WOOD];
        else if (slot.type == ITEM_IRON_PICKAXE) color = colorPalette[COLOR_CYAN];
        else if (slot.type == ITEM_DIAMOND) color = colorPalette[COLOR_DIAMOND];
        else if (slot.type == ITEM_DIAMOND_PICKAXE) color = colorPalette[COLOR_CYAN]; // Maybe a lighter cyan?

        glColor3fv(color);
        float item_inset = 5;
        glBegin(GL_QUADS);
        glVertex2f(x + item_inset, y + item_inset);
        glVertex2f(x + size - item_inset, y + item_inset);
        glVertex2f(x + size - item_inset, y + size - item_inset);
        glVertex2f(x + item_inset, y + size - item_inset);
        glEnd();

        // Draw diamond shape for diamond pickaxe
        if (slot.type == ITEM_DIAMOND_PICKAXE) {
            glColor3fv(colorPalette[COLOR_DIAMOND]);
            glBegin(GL_TRIANGLES);
                glVertex2f(x + size/2, y + item_inset + 2);
                glVertex2f(x + item_inset + 2, y + size/2);
                glVertex2f(x + size - item_inset -2, y + size/2);
            glEnd();
        }


        // Draw count text
        if (slot.count > 1) {
            char count_text[4];
            tiny_itoa(slot.count, count_text);
            glColor3f(1.0f, 1.0f, 1.0f);
            DrawText(x + size - 20, y + size - 15, count_text);
        }
    }
}


void RenderCraftingScreen() {
    glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 800, 600, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    DrawText(300, 50, "Altar of Crafting");

    // --- Draw Crafting Grid (3x3) ---
    const float slot_size = 50;
    const float padding = 10;
    const float grid_start_x = (800 - 3 * (slot_size + padding)) / 2.0f;
    const float grid_start_y = 150;

    // Draw center slot first
    DrawItemSlot(grid_start_x + slot_size + padding, grid_start_y + slot_size + padding, slot_size, crafting_grid[0], true);

    // Draw surrounding 8 slots
    int slot_index = 1;
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            if (x == 1 && y == 1) continue; // Skip center slot
            float slot_x = grid_start_x + x * (slot_size + padding);
            float slot_y = grid_start_y + y * (slot_size + padding);
            DrawItemSlot(slot_x, slot_y, slot_size, crafting_grid[slot_index++], false);
        }
    }

    // --- Draw Output Slot ---
    const float output_x = grid_start_x + 3 * (slot_size + padding) + 20;
    const float output_y = grid_start_y + slot_size + padding;
    DrawText(output_x - 10, output_y - 20, "Result");
    DrawItemSlot(output_x, output_y, slot_size, crafting_output, false);

    // Draw Arrow
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(grid_start_x + 3 * (slot_size + padding) - 10, output_y + slot_size / 2);
    glVertex2f(output_x - 5, output_y + slot_size / 2);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex2f(output_x - 5, output_y + slot_size / 2 + 8);
    glVertex2f(output_x - 5, output_y + slot_size / 2 - 8);
    glVertex2f(output_x + 3, output_y + slot_size / 2);
    glEnd();


    // --- Draw Player Inventory ---
    const float inv_start_x = (800 - INVENTORY_SLOTS * (slot_size + padding)) / 2.0f;
    const float inv_start_y = 450;
    DrawText(inv_start_x, inv_start_y - 20, "Inventory");
    for (int i = 0; i < INVENTORY_SLOTS; ++i) {
        DrawItemSlot(inv_start_x + i * (slot_size + padding), inv_start_y, slot_size, inventory[i], false);
    }

    // --- Draw Held Item (follows mouse) ---
    if (held_item.type != ITEM_NONE) {
        DrawItemSlot(mouseX - slot_size / 2, mouseY - slot_size / 2, slot_size, held_item, false);
    }

    glDisable(GL_BLEND);
    SwapBuffers(hDC);
}

// --- OpenGL Setup/Teardown ---
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