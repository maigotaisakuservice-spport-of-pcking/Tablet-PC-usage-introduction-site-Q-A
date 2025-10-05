#include "globals.h"
#include "game_logic.h"
#include "sound.h"
#include "save_system.h"
#include "utils.h"
#include "crafting.h"
#include <math.h>

// --- Lightweight Noise Implementation ---
unsigned int simple_hash(unsigned int x, unsigned int y) {
    unsigned int h = x * 374761393 + y * 668265263;
    h = (h ^ (h >> 13)) * 1274126177;
    return h ^ (h >> 16);
}

float value_noise(float x, float y) {
    int ix = floor(x); int iy = floor(y);
    float fx = x - ix; float fy = y - iy;
    unsigned int p00 = simple_hash(ix, iy); unsigned int p10 = simple_hash(ix + 1, iy);
    unsigned int p01 = simple_hash(ix, iy + 1); unsigned int p11 = simple_hash(ix + 1, iy + 1);
    float v00 = (p00 & 0xFFFF) / 65535.0f; float v10 = (p10 & 0xFFFF) / 65535.0f;
    float v01 = (p01 & 0xFFFF) / 65535.0f; float v11 = (p11 & 0xFFFF) / 65535.0f;
    float ix1 = v00 * (1.0f - fx) + v10 * fx;
    float ix2 = v01 * (1.0f - fx) + v11 * fx;
    return ix1 * (1.0f - fy) + ix2 * fy;
}

float octave_noise(float x, float y, int octaves, float persistence) {
    float total = 0, frequency = 1, amplitude = 1, maxValue = 0;
    for(int i=0; i<octaves; i++) {
        total += value_noise(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2;
    }
    return total/maxValue;
}

unsigned int simple_hash_3d(unsigned int x, unsigned int y, unsigned int z) {
    unsigned int h = x * 374761393 + y * 668265263 + z * 951274213;
    h = (h ^ (h >> 13)) * 1274126177;
    return h ^ (h >> 16);
}

float value_noise_3d(float x, float y, float z) {
    int ix = floor(x); int iy = floor(y); int iz = floor(z);
    float fx = x - ix; float fy = y - iy; float fz = z - iz;
    float v[8];
    for (int i = 0; i < 8; ++i) {
        v[i] = (simple_hash_3d(ix + (i & 1), iy + ((i >> 1) & 1), iz + ((i >> 2) & 1)) & 0xFFFF) / 65535.0f;
    }
    float ix1 = v[0] * (1.0f - fx) + v[1] * fx;
    float ix2 = v[2] * (1.0f - fx) + v[3] * fx;
    float ix3 = v[4] * (1.0f - fx) + v[5] * fx;
    float ix4 = v[6] * (1.0f - fx) + v[7] * fx;
    float iy1 = ix1 * (1.0f - fy) + ix2 * fy;
    float iy2 = ix3 * (1.0f - fy) + ix4 * fy;
    return iy1 * (1.0f - fz) + iy2 * fz;
}

float octave_noise_3d(float x, float y, float z, int octaves, float persistence) {
    float total = 0, frequency = 1, amplitude = 1, maxValue = 0;
    for(int i=0; i<octaves; i++) {
        total += value_noise_3d(x * frequency, y * frequency, z * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2;
    }
    return total/maxValue;
}

void GenerateTree(int x, int y, int z) {
    int height = 4 + (rand() % 3);
    for (int i = 0; i < height; ++i) {
        if (y + i < WORLD_HEIGHT) {
            int index = (y + i) * (WORLD_WIDTH * WORLD_DEPTH) + z * WORLD_WIDTH + x;
            worldData[index] = (1 << 7) | COLOR_WOOD;
        }
    }
    int leaf_radius = 2;
    for (int ly = height - 2; ly < height + 2; ++ly) {
        for (int lx = -leaf_radius; lx <= leaf_radius; ++lx) {
            for (int lz = -leaf_radius; lz <= leaf_radius; ++lz) {
                if (lx*lx + lz*lz <= leaf_radius*leaf_radius) {
                    int nx = x + lx;
                    int ny = y + ly;
                    int nz = z + lz;
                    if (nx >= 0 && nx < WORLD_WIDTH && ny >= 0 && ny < WORLD_HEIGHT && nz >= 0 && nz < WORLD_DEPTH) {
                        int index = ny * (WORLD_WIDTH * WORLD_DEPTH) + nz * WORLD_WIDTH + nx;
                        if (!(worldData[index] & (1 << 7))) {
                            worldData[index] = (1 << 7) | COLOR_LEAVES;
                        }
                    }
                }
            }
        }
    }
}

void InitializeGame() {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
        for (int z = 0; z < WORLD_DEPTH; ++z) {
            float temperature = octave_noise(x * 0.005f, z * 0.005f, 4, 0.5f);
            float humidity = octave_noise(x * 0.005f + 10000, z * 0.005f + 10000, 4, 0.5f);
            BiomeType biome;
            if (temperature > 0.6f && humidity < 0.4f) biome = BIOME_DESERT;
            else if (temperature > 0.4f && humidity > 0.5f) biome = BIOME_FOREST;
            else biome = BIOME_PLAINS;

            const float height_noise = octave_noise(x * 0.015f, z * 0.015f, 5, 0.5f);
            const int terrainHeight = static_cast<int>(height_noise * (WORLD_HEIGHT * 0.75));

            for (int y = 0; y < WORLD_HEIGHT; ++y) {
                int index = y * (WORLD_WIDTH * WORLD_DEPTH) + z * WORLD_WIDTH + x;
                if (y < terrainHeight) {
                    float cave_noise = octave_noise_3d(x * 0.05f, y * 0.05f, z * 0.05f, 6, 0.5f);
                    if (cave_noise > 0.6f) {
                        worldData[index] = 0;
                    } else {
                        if (y == terrainHeight - 1) {
                            if (biome == BIOME_PLAINS || biome == BIOME_FOREST) {
                                worldData[index] = (1 << 7) | COLOR_GRASS;
                                if (biome == BIOME_FOREST && (rand() % 100) < 5) {
                                    GenerateTree(x, y + 1, z);
                                }
                            }
                            else if (biome == BIOME_DESERT) worldData[index] = (1 << 7) | COLOR_SAND;
                        } else if (y > terrainHeight - 5) {
                             if (biome == BIOME_DESERT) worldData[index] = (1 << 7) | COLOR_SANDSTONE;
                             else worldData[index] = (1 << 7) | COLOR_DIRT;
                        }
                        else {
                            worldData[index] = (1 << 7) | COLOR_GRAY;
                        }
                    }
                } else {
                    worldData[index] = 0;
                }
            }
        }
    }
    for(int i=0; i<MAX_DRONES; ++i) {
        drones[i].active = false;
        drones[i].anim.step_cycle = 0.0f;
        drones[i].anim.swing_speed = 0.1f + (rand() % 50 / 100.0f);
        drones[i].anim.swing_angle = 0.0f; // Drones don't swing, they spin
    }
    for(int i=0; i<MAX_ZOMBIES; ++i) {
        zombies[i].active = false;
        zombies[i].anim.step_cycle = 0.0f;
        zombies[i].anim.swing_angle = 45.0f; // Zombies swing arms by 45 degrees
    }
    for (int i = 0; i < MAX_PIGS; ++i) pigs[i].active = false;
    for (int i = 0; i < MAX_COWS; ++i) cows[i].active = false;

    boss.active = false;
    boss.hp = 50;
    boss.aoe_effect_timer = 0;
    for (int i = 0; i < MAX_PROJECTILES; ++i) projectiles[i].active = false;

    playerHP = 20;
    score = 0;
    tiny_strcpy(currentWorldName, "");

    InitializeInventory();
    InitializeRecipes();
    AddItemToInventory(ITEM_IRON_PICKAXE, 1);
    AddItemToInventory(ITEM_DIAMOND, 5); // Give player some diamonds for testing
}

void UpdateGame() {
    gameTime += 0.0005f;
    sunDirection[0] = cos(gameTime);
    sunDirection[1] = 0.8f + sin(gameTime) * 0.4f;
    sunDirection[2] = sin(gameTime);

    if (gameMode == MODE_NORMAL) {
        static int weather_timer = 0;
        if (++weather_timer > 6000) {
            weather_timer = 0;
            int next_weather = (int)currentWeather + 1;
            currentWeather = (WeatherType)(next_weather > 2 ? 0 : next_weather);
        }
    }

    if (currentWeather != WEATHER_CLEAR) {
        for (int i = 0; i < 10; ++i) {
             for (int p = 0; p < MAX_PARTICLES; ++p) {
                if (!particles[p].active) {
                    particles[p].active = true;
                    particles[p].x = camX + (rand() % 40 - 20);
                    particles[p].y = camY + 15.0f;
                    particles[p].z = camZ + (rand() % 40 - 20);
                    if (currentWeather == WEATHER_RAIN) {
                        particles[p].vy = -0.5f; particles[p].vx = 0; particles[p].vz = 0;
                    } else {
                        particles[p].vy = -0.05f; particles[p].vx = (rand() % 100 / 100.0f - 0.5f) * 0.05f; particles[p].vz = (rand() % 100 / 100.0f - 0.5f) * 0.05f;
                    }
                    break;
                }
            }
        }
    }

    for (int p = 0; p < MAX_PARTICLES; ++p) {
        if (particles[p].active) {
            particles[p].x += particles[p].vx;
            particles[p].y += particles[p].vy;
            particles[p].z += particles[p].vz;
            if (particles[p].y < 0) particles[p].active = false;
        }
    }

    if (gameMode == MODE_NORMAL) {
        if (rand() % 1000 < 5) {
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
        if (sunDirection[1] < 0.6f && rand() % 1000 < 10) {
            for (int i = 0; i < MAX_ZOMBIES; ++i) {
                if (!zombies[i].active) {
                    zombies[i].active = true;
                    zombies[i].hp = 5;
                    zombies[i].x = camX + (rand() % 30 - 15);
                    zombies[i].y = camY - 2.0f;
                    zombies[i].z = camZ + (rand() % 30 - 15);
                    zombies[i].anim.swing_speed = 0.05f + (rand() % 20 / 100.0f);
                    break;
                }
            }
        }

        for(int i=0; i<MAX_DRONES; ++i) {
            if(drones[i].active) {
                float dx = camX - drones[i].x; float dy = camY - drones[i].y; float dz = camZ - drones[i].z;
                float dist = sqrt(dx*dx + dy*dy + dz*dz);
                if (dist > 0) {
                    drones[i].x += (dx / dist) * 0.05f;
                    drones[i].y += (dy / dist) * 0.05f;
                    drones[i].z += (dz / dist) * 0.05f;
                }
                drones[i].anim.step_cycle += drones[i].anim.swing_speed; // Update animation timer
            }
        }

        for(int i=0; i<MAX_ZOMBIES; ++i) {
            if(zombies[i].active) {
                float dx = camX - zombies[i].x; float dy = camY - zombies[i].y; float dz = camZ - zombies[i].z;
                float dist = sqrt(dx*dx + dz*dz);
                if (dist > 0) {
                    zombies[i].x += (dx / dist) * 0.03f;
                    zombies[i].z += (dz / dist) * 0.03f;
                }
                zombies[i].anim.step_cycle += zombies[i].anim.swing_speed; // Update animation timer

                if (dist < 1.5f) {
                    playerHP--;
                    PlaySoundEffect(120, 100);
                    if (playerHP <= 0) PostQuitMessage(0);
                    zombies[i].x -= (dx / dist) * 0.5f;
                    zombies[i].z -= (dz / dist) * 0.5f;
                }
            }
        }

        if (boss.active) {
            if (boss.aoe_effect_timer > 0) boss.aoe_effect_timer--;
            float dx = camX - boss.x; float dy = camY - boss.y; float dz = camZ - boss.z;
            float dist = sqrt(dx*dx + dy*dy + dz*dz);
            if (dist > 0) {
                boss.x += (dx / dist) * 0.02f;
                boss.y += (dy / dist) * 0.02f;
                boss.z += (dz / dist) * 0.02f;
            }
            if (--boss.action_timer <= 0) {
                int action = rand() % 4;
                if (action == 0) {
                    boss.x = camX + (rand() % 40 - 20);
                    boss.y = camY + (rand() % 10);
                    boss.z = camZ + (rand() % 40 - 20);
                    PlaySoundEffect(100, 100);
                    boss.action_timer = 150 + (rand() % 100);
                } else if (action == 1) {
                     for (int i = 0; i < MAX_PROJECTILES; ++i) {
                        if (!projectiles[i].active) {
                            projectiles[i].active = true;
                            projectiles[i].x = boss.x; projectiles[i].y = boss.y; projectiles[i].z = boss.z;
                            float p_dx = camX - boss.x, p_dy = camY - boss.y, p_dz = camZ - boss.z;
                            float p_dist = sqrt(p_dx*p_dx + p_dy*p_dy + p_dz*p_dz);
                            projectiles[i].vx = (p_dx / p_dist) * 0.2f;
                            projectiles[i].vy = (p_dy / p_dist) * 0.2f;
                            projectiles[i].vz = (p_dz / p_dist) * 0.2f;
                            PlaySoundEffect(400, 100);
                            break;
                        }
                    }
                    boss.action_timer = 100 + (rand() % 50);
                } else if (action == 2) {
                    const float aoe_radius = 10.0f;
                    float dx_aoe = camX - boss.x, dy_aoe = camY - boss.y, dz_aoe = camZ - boss.z;
                    if ((dx_aoe*dx_aoe + dy_aoe*dy_aoe + dz_aoe*dz_aoe) < aoe_radius * aoe_radius) {
                        playerHP -= 5;
                        PlaySoundEffect(100, 150);
                        if (playerHP <= 0) PostQuitMessage(0);
                    }
                    boss.aoe_effect_timer = 50;
                    PlaySoundEffect(50, 200);
                    boss.action_timer = 200 + (rand() % 100);
                } else {
                     boss.action_timer = 80 + (rand() % 50);
                }
            }
        }
    }

    if (rand() % 1500 < 5) {
        for (int i = 0; i < MAX_PIGS; ++i) {
            if (!pigs[i].active) {
                pigs[i].active = true;
                pigs[i].x = camX + (rand() % 40 - 20);
                pigs[i].y = camY - 2.0f;
                pigs[i].z = camZ + (rand() % 40 - 20);
                pigs[i].wander_timer = 0;
                pigs[i].anim.swing_speed = 0.08f + (rand() % 10 / 100.0f);
                pigs[i].anim.swing_angle = 25.0f;
                pigs[i].anim.step_cycle = 0.0f;
                break;
            }
        }
    }

    for (int i = 0; i < MAX_PIGS; ++i) {
        if (pigs[i].active) {
            if (--pigs[i].wander_timer <= 0) {
                float angle = (rand() % 360) * 3.14159 / 180.0;
                float speed = 0.02f;
                pigs[i].vx = cos(angle) * speed;
                pigs[i].vz = sin(angle) * speed;
                pigs[i].wander_timer = 100 + (rand() % 200);
            }
            pigs[i].x += pigs[i].vx;
            pigs[i].z += pigs[i].vz;
            float speed = sqrt(pigs[i].vx*pigs[i].vx + pigs[i].vz*pigs[i].vz);
            if (speed > 0.01f) {
                 pigs[i].anim.step_cycle += pigs[i].anim.swing_speed;
            }
        }
    }

    if (rand() % 1500 < 5) {
        for (int i = 0; i < MAX_COWS; ++i) {
            if (!cows[i].active) {
                cows[i].active = true;
                cows[i].x = camX + (rand() % 50 - 25);
                cows[i].y = camY - 2.0f;
                cows[i].z = camZ + (rand() % 50 - 25);
                cows[i].wander_timer = 0;
                cows[i].anim.swing_speed = 0.06f + (rand() % 10 / 100.0f);
                cows[i].anim.swing_angle = 20.0f;
                cows[i].anim.step_cycle = 0.0f;
                break;
            }
        }
    }

    for (int i = 0; i < MAX_COWS; ++i) {
        if (cows[i].active) {
            if (--cows[i].wander_timer <= 0) {
                float angle = (rand() % 360) * 3.14159 / 180.0;
                float speed = 0.015f;
                cows[i].vx = cos(angle) * speed;
                cows[i].vz = sin(angle) * speed;
                cows[i].wander_timer = 150 + (rand() % 250);
            }
            cows[i].x += cows[i].vx;
            cows[i].z += cows[i].vz;
            float speed = sqrt(cows[i].vx*cows[i].vx + cows[i].vz*cows[i].vz);
            if (speed > 0.01f) {
                 cows[i].anim.step_cycle += cows[i].anim.swing_speed;
            }
        }
    }

    for (int i = 0; i < MAX_PROJECTILES; ++i) {
        if (projectiles[i].active) {
            projectiles[i].x += projectiles[i].vx;
            projectiles[i].y += projectiles[i].vy;
            projectiles[i].z += projectiles[i].vz;
            if (gameMode == MODE_NORMAL && fabs(projectiles[i].x - camX) < 0.5f && fabs(projectiles[i].y - camY) < 0.5f && fabs(projectiles[i].z - camZ) < 0.5f) {
                projectiles[i].active = false;
                playerHP--;
                PlaySoundEffect(150, 80);
                if (playerHP <= 0) PostQuitMessage(0);
            } else if (abs(projectiles[i].x - camX) > 50 || abs(projectiles[i].y - camY) > 50 || abs(projectiles[i].z - camZ) > 50) {
                projectiles[i].active = false;
            }
        }
    }
}

void UpdatePhysics() {
    for (int i = 0; i < 250; ++i) {
        int x = rand() % WORLD_WIDTH;
        int z = rand() % WORLD_DEPTH;
        for (int y = 1; y < WORLD_HEIGHT; ++y) {
            int currentIndex = y * (WORLD_WIDTH * WORLD_DEPTH) + z * WORLD_WIDTH + x;
            unsigned char currentBlock = worldData[currentIndex];
            if (currentBlock & (1 << 7)) {
                int belowIndex = (y - 1) * (WORLD_WIDTH * WORLD_DEPTH) + z * WORLD_WIDTH + x;
                if (!(worldData[belowIndex] & (1 << 7))) {
                    worldData[belowIndex] = currentBlock;
                    worldData[currentIndex] = 0;
                }
            }
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            break;
        case WM_KEYDOWN: {
            if (gameState != STATE_GAME) break;
            float speed = 0.2f;
            float yawRad = camYaw * 3.14159265f / 180.0f;
            float forwardX = sin(yawRad); float forwardZ = -cos(yawRad);
            switch(wParam) {
                case 'W': camX += forwardX * speed; camZ += forwardZ * speed; break;
                case 'S': camX -= forwardX * speed; camZ -= forwardZ * speed; break;
                case 'A': camX -= forwardZ * speed; camZ += forwardX * speed; break;
                case 'D': camX += forwardZ * speed; camZ -= forwardX * speed; break;
                case 'B': isSphereMode = !isSphereMode; break;
                case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                    currentColorIndex = wParam - '1';
                    break;
                case 'P':
                    if (gameState == STATE_GAME) SaveWorld(currentWorldName);
                    break;
                case 'C':
                    if (gameState == STATE_GAME) {
                        gameState = STATE_CRAFTING;
                        ShowCursor(TRUE);
                    } else if (gameState == STATE_CRAFTING) {
                        gameState = STATE_GAME;
                        ShowCursor(FALSE);
                        SetCursorPos(400, 300);
                    }
                    break;
                case VK_ESCAPE:
                    if (gameState == STATE_CRAFTING) {
                        gameState = STATE_GAME;
                        ShowCursor(FALSE);
                        SetCursorPos(400, 300);
                    } else {
                        PostQuitMessage(0);
                    }
                    break;
            }
        } break;
        case WM_MOUSEMOVE: {
            mouseX = LOWORD(lParam);
            mouseY = HIWORD(lParam);

            if (gameState == STATE_TITLE) {
                if (mouseY > 240 && mouseY < 280) selectedMenuItem = 0;
                else if (mouseY > 290 && mouseY < 330) selectedMenuItem = 1;
                else if (mouseY > 340 && mouseY < 380) selectedMenuItem = 2;
                else if (mouseY > 390 && mouseY < 430) selectedMenuItem = 3;
            } else if (gameState == STATE_SETTINGS) {
                if (mouseY > 240 && mouseY < 280) selectedMenuItem = 0;
                else if (mouseY > 390 && mouseY < 430) selectedMenuItem = 1;
            } else if (gameState == STATE_WORLD_SELECT) {
                int item_count = world_count < 5 ? world_count + 1 : 5;
                selectedMenuItem = -1;
                for(int i = 0; i < item_count; ++i) {
                    if (mouseY > 190 + i * 40 && mouseY < 230 + i * 40) selectedMenuItem = i;
                }
                if (mouseY > 440 && mouseY < 480) selectedMenuItem = 5;
            }
            else if (gameState == STATE_GAME) {
                int deltaX = mouseX - 400; int deltaY = mouseY - 300;
                camYaw += deltaX * 0.1f; camPitch -= deltaY * 0.1f;
                if(camPitch > 89.0f) camPitch = 89.0f;
                if(camPitch < -89.0f) camPitch = -89.0f;
                SetCursorPos(400, 300);
            }
        } break;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN: {
            if (uMsg == WM_LBUTTONDOWN && gameState == STATE_TITLE) {
                if (selectedMenuItem == 0) {
                    gameMode = MODE_NORMAL;
                    ScanForWorlds();
                    gameState = STATE_WORLD_SELECT;
                    selectedMenuItem = 0;
                } else if (selectedMenuItem == 1) {
                    gameMode = MODE_PEACEFUL;
                    ScanForWorlds();
                    gameState = STATE_WORLD_SELECT;
                    selectedMenuItem = 0;
                } else if (selectedMenuItem == 2) {
                    gameState = STATE_SETTINGS;
                    selectedMenuItem = 0;
                } else if (selectedMenuItem == 3) {
                    PostQuitMessage(0);
                }
            } else if (uMsg == WM_LBUTTONDOWN && gameState == STATE_WORLD_SELECT) {
                if (selectedMenuItem >= 0 && selectedMenuItem < world_count) {
                    tiny_strcpy(currentWorldName, world_names[selectedMenuItem]);
                    LoadWorld(currentWorldName);
                    gameState = STATE_GAME;
                } else if (selectedMenuItem == world_count && world_count < 5) {
                    tiny_sprintf_world(currentWorldName, world_count + 1);
                    InitializeGame();
                    SaveWorld(currentWorldName);
                    gameState = STATE_GAME;
                } else if (selectedMenuItem == 5) {
                    gameState = STATE_TITLE;
                    selectedMenuItem = 0;
                }
            } else if (uMsg == WM_LBUTTONDOWN && gameState == STATE_SETTINGS) {
                if (selectedMenuItem == 0) {
                    musicEnabled = !musicEnabled;
                    if (musicEnabled) StartMusicThread();
                    else StopMusicThread();
                } else if (selectedMenuItem == 1) {
                    gameState = STATE_TITLE;
                    selectedMenuItem = 0;
                }
            } else if (gameState == STATE_CRAFTING) {
                // --- Crafting UI Click Logic ---
                const float slot_size = 50;
                const float padding = 10;
                bool clicked_on_slot = false;

                // Check crafting grid slots
                const float grid_start_x = (800 - 3 * (slot_size + padding)) / 2.0f;
                const float grid_start_y = 150;
                int slot_index = 1;
                 for (int y = 0; y < 3; ++y) {
                    for (int x = 0; x < 3; ++x) {
                        float slot_x = grid_start_x + x * (slot_size + padding);
                        float slot_y = grid_start_y + y * (slot_size + padding);
                        int current_slot_idx = (x == 1 && y == 1) ? 0 : slot_index++;

                        if (mouseX > slot_x && mouseX < slot_x + slot_size && mouseY > slot_y && mouseY < slot_y + slot_size) {
                            InventorySlot* target_slot = &crafting_grid[current_slot_idx];
                             // SWAP logic
                            InventorySlot temp = *target_slot;
                            *target_slot = held_item;
                            held_item = temp;
                            clicked_on_slot = true;
                            break;
                        }
                    }
                    if(clicked_on_slot) break;
                }

                // Check inventory slots
                if (!clicked_on_slot) {
                    const float inv_start_x = (800 - INVENTORY_SLOTS * (slot_size + padding)) / 2.0f;
                    const float inv_start_y = 450;
                    for (int i = 0; i < INVENTORY_SLOTS; ++i) {
                        float slot_x = inv_start_x + i * (slot_size + padding);
                        if (mouseX > slot_x && mouseX < slot_x + slot_size && mouseY > inv_start_y && mouseY < inv_start_y + slot_size) {
                             InventorySlot* target_slot = &inventory[i];
                             // SWAP logic
                            InventorySlot temp = *target_slot;
                            *target_slot = held_item;
                            held_item = temp;
                            clicked_on_slot = true;
                            break;
                        }
                    }
                }

                // Check output slot
                if (!clicked_on_slot) {
                     const float output_x = grid_start_x + 3 * (slot_size + padding) + 20;
                     const float output_y = grid_start_y + slot_size + padding;
                     if (mouseX > output_x && mouseX < output_x + slot_size && mouseY > output_y && mouseY < output_y + slot_size) {
                         if (crafting_output.type != ITEM_NONE && held_item.type == ITEM_NONE) {
                             DoCraft(); // This moves the item to held_item
                         }
                         clicked_on_slot = true;
                     }
                }

                CheckRecipe(); // Check for a valid recipe after any click action

            } else if (gameState == STATE_GAME) {
                float rayX = camX, rayY = camY, rayZ = camZ;
                float pitchRad = camPitch * 3.14159 / 180.0; float yawRad = camYaw * 3.14159 / 180.0;
                float dirX = sin(yawRad) * cos(pitchRad); float dirY = -sin(pitchRad); float dirZ = -cos(yawRad) * cos(pitchRad);
                int lastX=0, lastY=0, lastZ=0;
                for (float step = 0; step < 8.0f; step += 0.05f) {
                    lastX = floor(rayX); lastY = floor(rayY); lastZ = floor(rayZ);
                    rayX += dirX * 0.05f; rayY += dirY * 0.05f; rayZ += dirZ * 0.05f;
                    if (uMsg == WM_LBUTTONDOWN) {
                        if (gameMode == MODE_NORMAL && boss.active) {
                            if (rayX > boss.x - 1.2f && rayX < boss.x + 1.2f && rayY > boss.y - 1.2f && rayY < boss.y + 1.2f && rayZ > boss.z - 1.2f && rayZ < boss.z + 1.2f) {
                                boss.hp--;
                                PlaySoundEffect(200, 50);
                                if (boss.hp <= 0) {
                                    boss.active = false;
                                    gameMode = MODE_PEACEFUL;
                                    currentWeather = WEATHER_CLEAR;
                                    playerHP = 20;
                                    PlaySoundEffect(523, 200); Sleep(250); PlaySoundEffect(659, 200); Sleep(250);
                                    PlaySoundEffect(783, 200); Sleep(250); PlaySoundEffect(1046, 400);
                                }
                                goto raycast_end;
                            }
                        }
                        if (gameMode == MODE_NORMAL) {
                            for (int i = 0; i < MAX_ZOMBIES; ++i) if (zombies[i].active) {
                                if (rayX > zombies[i].x - 0.5f && rayX < zombies[i].x + 0.5f && rayY > zombies[i].y - 1.0f && rayY < zombies[i].y + 1.0f && rayZ > zombies[i].z - 0.5f && rayZ < zombies[i].z + 0.5f) {
                                    zombies[i].hp--;
                                    PlaySoundEffect(250, 50);
                                    if (zombies[i].hp <= 0) {
                                        zombies[i].active = false;
                                        score += 5;
                                    }
                                    goto raycast_end;
                                }
                            }
                            for (int i = 0; i < MAX_DRONES; ++i) if (drones[i].active) {
                                if ((pow(drones[i].x - rayX, 2) + pow(drones[i].y - rayY, 2) + pow(drones[i].z - rayZ, 2)) < 1.0f) {
                                    drones[i].active = false; score++; PlaySoundEffect(150, 150);
                                    goto raycast_end;
                                }
                            }
                        }
                    }
                    int blockX = floor(rayX), blockY = floor(rayY), blockZ = floor(rayZ);
                    if (blockX < 0 || blockX >= WORLD_WIDTH || blockY < 0 || blockY >= WORLD_HEIGHT || blockZ < 0 || blockZ >= WORLD_DEPTH) break;
                    int index = blockY * (WORLD_WIDTH * WORLD_DEPTH) + blockZ * WORLD_WIDTH + blockX;
                    if (worldData[index] & (1 << 7)) {
                        if (uMsg == WM_LBUTTONDOWN) {
                            unsigned char blockType = worldData[index] & 0x0F;
                            if (blockType == COLOR_DIRT || blockType == COLOR_GRASS) AddItemToInventory(ITEM_DIRT, 1);
                            else if (blockType == COLOR_GRAY) AddItemToInventory(ITEM_STONE, 1);
                            else if (blockType == COLOR_WOOD) AddItemToInventory(ITEM_WOOD, 1);

                            worldData[index] = 0;
                            PlaySoundEffect(300, 50);
                        } else if (uMsg == WM_RBUTTONDOWN) {
                            int prevIndex = lastY * (WORLD_WIDTH * WORLD_DEPTH) + lastZ * WORLD_WIDTH + lastX;
                            if (lastX >= 0 && lastX < WORLD_WIDTH && lastY >= 0 && lastY < WORLD_HEIGHT && lastZ >= 0 && lastZ < WORLD_DEPTH) {
                                unsigned char blockData = (1 << 7) | (isSphereMode ? (1 << 4) : 0) | (currentColorIndex & 0x0F);
                                worldData[prevIndex] = blockData;
                                PlaySoundEffect(600, 50);
                                if (gameMode == MODE_NORMAL && !boss.active && !isSphereMode && currentColorIndex == 4 && lastY > 0) {
                                    int belowIndex = (lastY - 1) * (WORLD_WIDTH * WORLD_DEPTH) + lastZ * WORLD_WIDTH + lastX;
                                    unsigned char belowBlockData = worldData[belowIndex];
                                    if ((belowBlockData & 0x9F) == ((1 << 7) | 7)) {
                                        boss.active = true;
                                        boss.hp = 50;
                                        boss.action_timer = 100;
                                        boss.x = lastX;
                                        boss.y = lastY + 5;
                                        boss.z = lastZ;
                                        PlaySoundEffect(50, 500);
                                        currentWeather = WEATHER_RAIN;
                                    }
                                }
                            }
                        } else {
                            const int blast_radius = 4;
                            for (int dx = -blast_radius; dx <= blast_radius; ++dx)
                            for (int dy = -blast_radius; dy <= blast_radius; ++dy)
                            for (int dz = -blast_radius; dz <= blast_radius; ++dz)
                                if (dx*dx + dy*dy + dz*dz <= blast_radius*blast_radius) {
                                    int effectX = blockX + dx, effectY = blockY + dy, effectZ = blockZ + dz;
                                    if (effectX >= 0 && effectX < WORLD_WIDTH && effectY >= 0 && effectY < WORLD_HEIGHT && effectZ >= 0 && effectZ < WORLD_DEPTH) {
                                        int effectIndex = effectY * (WORLD_WIDTH * WORLD_DEPTH) + effectZ * WORLD_WIDTH + effectX;
                                        if (worldData[effectIndex] & (1 << 7)) worldData[effectIndex] = 0;
                                    }
                                }
                            PlaySoundEffect(100, 200);
                        }
                        break;
                    }
                }
                raycast_end:;
            }
        } break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}