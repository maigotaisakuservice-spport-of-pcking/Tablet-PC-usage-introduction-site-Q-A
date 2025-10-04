#include "game_logic.h"
#include "globals.h"
#include "sound.h"
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
// --- End Noise ---

void InitializeGame() {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
        for (int z = 0; z < WORLD_DEPTH; ++z) {
            const float noise = octave_noise(x * 0.015f, z * 0.015f, 5, 0.5f);
            const int terrainHeight = static_cast<int>(noise * (WORLD_HEIGHT * 0.75));
            for (int y = 0; y < WORLD_HEIGHT; ++y) {
                if (y < terrainHeight) {
                    worldData[y * (WORLD_WIDTH * WORLD_DEPTH) + z * WORLD_WIDTH + x] = (1 << 7) | 8;
                } else {
                    worldData[y * (WORLD_WIDTH * WORLD_DEPTH) + z * WORLD_WIDTH + x] = 0;
                }
            }
        }
    }
    for(int i=0; i<MAX_DRONES; ++i) {
        drones[i].active = false;
        drones[i].anim.swing_angle = 0;
        drones[i].anim.swing_speed = 0.1f + (rand() % 50 / 100.0f);
    }
    for(int i=0; i<MAX_ZOMBIES; ++i) zombies[i].active = false;

    boss.active = false;
    boss.hp = 50;
    boss.aoe_effect_timer = 0;
    for (int i = 0; i < MAX_PROJECTILES; ++i) projectiles[i].active = false;
    for (int i = 0; i < MAX_PIGS; ++i) pigs[i].active = false;
    for (int i = 0; i < MAX_COWS; ++i) cows[i].active = false;
    playerHP = 20;
    score = 0;
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
        // Drone Spawning
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
        // Zombie Spawning (at night)
        if (sunDirection[1] < 0.6f && rand() % 1000 < 10) {
            for (int i = 0; i < MAX_ZOMBIES; ++i) {
                if (!zombies[i].active) {
                    zombies[i].active = true;
                    zombies[i].hp = 5;
                    zombies[i].x = camX + (rand() % 30 - 15);
                    zombies[i].y = camY - 2.0f; // Spawn on the ground
                    zombies[i].z = camZ + (rand() % 30 - 15);
                    zombies[i].anim.swing_speed = 0.05f + (rand() % 20 / 100.0f);
                    break;
                }
            }
        }

        // --- Update Animations and AI ---
        for(int i=0; i<MAX_DRONES; ++i) {
            if(drones[i].active) {
                // AI
                float dx = camX - drones[i].x; float dy = camY - drones[i].y; float dz = camZ - drones[i].z;
                float dist = sqrt(dx*dx + dy*dy + dz*dz);
                if (dist > 0) {
                    float speed = 0.05f;
                    drones[i].x += (dx / dist) * speed;
                    drones[i].y += (dy / dist) * speed;
                    drones[i].z += (dz / dist) * speed;
                }
                // Animation
                drones[i].anim.swing_angle += drones[i].anim.swing_speed * 20.0f;
            }
        }

        for(int i=0; i<MAX_ZOMBIES; ++i) {
            if(zombies[i].active) {
                // AI
                float dx = camX - zombies[i].x; float dy = camY - zombies[i].y; float dz = camZ - zombies[i].z;
                float dist = sqrt(dx*dx + dz*dz); // Ignore Y for 2D distance
                if (dist > 0) {
                    float speed = 0.03f;
                    zombies[i].x += (dx / dist) * speed;
                    zombies[i].z += (dz / dist) * speed;
                }
                // Animation
                zombies[i].anim.step_cycle += zombies[i].anim.swing_speed;
                zombies[i].anim.swing_angle = sin(zombies[i].anim.step_cycle) * 45.0f;

            // Attack player if close
            if (dist < 1.5f) {
                playerHP--;
                PlaySoundEffect(120, 100); // Zombie attack sound
                if (playerHP <= 0) PostQuitMessage(0);
                // Knockback the zombie slightly
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
                float speed = 0.02f;
                boss.x += (dx / dist) * speed;
                boss.y += (dy / dist) * speed;
                boss.z += (dz / dist) * speed;
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

    // --- Update Passive Mobs (Pigs) ---
    // Spawning
    if (rand() % 1000 < 5) {
        for (int i = 0; i < MAX_PIGS; ++i) {
            if (!pigs[i].active) {
                pigs[i].active = true;
                pigs[i].x = camX + (rand() % 40 - 20);
                pigs[i].y = camY - 2.0f; // Spawn on ground
                pigs[i].z = camZ + (rand() % 40 - 20);
                pigs[i].wander_timer = 0;
                pigs[i].anim.swing_speed = 0.04f + (rand() % 10 / 100.0f);
                break;
            }
        }
    }

    // AI and Animation
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

            // Simple animation based on movement
            float speed = sqrt(pigs[i].vx*pigs[i].vx + pigs[i].vz*pigs[i].vz);
            if (speed > 0.01f) {
                 pigs[i].anim.step_cycle += pigs[i].anim.swing_speed * 5.0f;
                 pigs[i].anim.swing_angle = sin(pigs[i].anim.step_cycle) * 25.0f;
            } else {
                pigs[i].anim.swing_angle = 0;
            }
        }
    }

    // Spawning Cows
    if (rand() % 1500 < 5) { // Cows are a bit rarer
        for (int i = 0; i < MAX_COWS; ++i) {
            if (!cows[i].active) {
                cows[i].active = true;
                cows[i].x = camX + (rand() % 50 - 25);
                cows[i].y = camY - 2.0f;
                cows[i].z = camZ + (rand() % 50 - 25);
                cows[i].wander_timer = 0;
                cows[i].anim.swing_speed = 0.03f + (rand() % 10 / 100.0f);
                break;
            }
        }
    }

    // AI and Animation for Cows
    for (int i = 0; i < MAX_COWS; ++i) {
        if (cows[i].active) {
            if (--cows[i].wander_timer <= 0) {
                float angle = (rand() % 360) * 3.14159 / 180.0;
                float speed = 0.015f; // Slower than pigs
                cows[i].vx = cos(angle) * speed;
                cows[i].vz = sin(angle) * speed;
                cows[i].wander_timer = 150 + (rand() % 250);
            }
            cows[i].x += cows[i].vx;
            cows[i].z += cows[i].vz;

            float speed = sqrt(cows[i].vx*cows[i].vx + cows[i].vz*cows[i].vz);
            if (speed > 0.01f) {
                 cows[i].anim.step_cycle += cows[i].anim.swing_speed * 5.0f;
                 cows[i].anim.swing_angle = sin(cows[i].anim.step_cycle) * 25.0f;
            } else {
                cows[i].anim.swing_angle = 0;
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
                case VK_ESCAPE: PostQuitMessage(0); break;
            }
        } break;
        case WM_MOUSEMOVE: {
            if (gameState == STATE_TITLE) {
                int yPos = HIWORD(lParam);
                if (yPos > 240 && yPos < 280) selectedMenuItem = 0;
                else if (yPos > 290 && yPos < 330) selectedMenuItem = 1;
                else if (yPos > 340 && yPos < 380) selectedMenuItem = 2;
                else if (yPos > 390 && yPos < 430) selectedMenuItem = 3;
            } else if (gameState == STATE_SETTINGS) {
                int yPos = HIWORD(lParam);
                if (yPos > 240 && yPos < 280) selectedMenuItem = 0;
                else if (yPos > 390 && yPos < 430) selectedMenuItem = 1;
            } else if (gameState == STATE_GAME) {
                int xPos = LOWORD(lParam); int yPos = HIWORD(lParam);
                int deltaX = xPos - 400; int deltaY = yPos - 300;
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
                if (selectedMenuItem == 0) { // Survival
                    gameMode = MODE_NORMAL;
                    gameState = STATE_GAME;
                    InitializeGame();
                } else if (selectedMenuItem == 1) { // Creative
                    gameMode = MODE_PEACEFUL;
                    gameState = STATE_GAME;
                    InitializeGame();
                } else if (selectedMenuItem == 2) { // Settings
                    gameState = STATE_SETTINGS;
                    selectedMenuItem = 0; // Reset for settings menu
                } else if (selectedMenuItem == 3) { // Exit
                    PostQuitMessage(0);
                }
            } else if (uMsg == WM_LBUTTONDOWN && gameState == STATE_SETTINGS) {
                if (selectedMenuItem == 0) { // Toggle Music
                    musicEnabled = !musicEnabled;
                    if (musicEnabled) StartMusicThread();
                    else StopMusicThread();
                } else if (selectedMenuItem == 1) { // Back
                    gameState = STATE_TITLE;
                    selectedMenuItem = 0; // Reset for title menu
                }
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
                        for (int i = 0; i < MAX_ZOMBIES; ++i) if (zombies[i].active) {
                            if (rayX > zombies[i].x - 0.5f && rayX < zombies[i].x + 0.5f &&
                                rayY > zombies[i].y - 1.0f && rayY < zombies[i].y + 1.0f &&
                                rayZ > zombies[i].z - 0.5f && rayZ < zombies[i].z + 0.5f) {

                                zombies[i].hp--;
                                PlaySoundEffect(250, 50); // Zombie hit sound
                                if (zombies[i].hp <= 0) {
                                    zombies[i].active = false;
                                    score += 5;
                                }
                                goto raycast_end;
                            }
                        }
                        if (gameMode == MODE_NORMAL) {
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
                            worldData[index] = 0; PlaySoundEffect(300, 50);
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