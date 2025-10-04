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
    // Generate world
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
    // Initialize drones
    for(int i=0; i<MAX_DRONES; ++i) drones[i].active = false;
}

void UpdateGame() {
    // Update game time and sun direction
    gameTime += 0.0005f; // Controls the speed of the day-night cycle
    sunDirection[0] = cos(gameTime);
    sunDirection[1] = 0.8f + sin(gameTime) * 0.4f; // Sun rises and sets
    sunDirection[2] = sin(gameTime);

    // Drone spawning
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
    // Drone AI
    for(int i=0; i<MAX_DRONES; ++i) {
        if(drones[i].active) {
            float dx = camX - drones[i].x; float dy = camY - drones[i].y; float dz = camZ - drones[i].z;
            float dist = sqrt(dx*dx + dy*dy + dz*dz);
            if (dist > 0) {
                float speed = 0.05f;
                drones[i].x += (dx / dist) * speed;
                drones[i].y += (dy / dist) * speed;
                drones[i].z += (dz / dist) * speed;
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
            int xPos = LOWORD(lParam); int yPos = HIWORD(lParam);
            int deltaX = xPos - 400; int deltaY = yPos - 300;
            camYaw += deltaX * 0.1f; camPitch -= deltaY * 0.1f;
            if(camPitch > 89.0f) camPitch = 89.0f;
            if(camPitch < -89.0f) camPitch = -89.0f;
            SetCursorPos(400, 300);
        } break;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN: {
            float rayX = camX, rayY = camY, rayZ = camZ;
            float pitchRad = camPitch * 3.14159 / 180.0; float yawRad = camYaw * 3.14159 / 180.0;
            float dirX = sin(yawRad) * cos(pitchRad); float dirY = -sin(pitchRad); float dirZ = -cos(yawRad) * cos(pitchRad);
            int lastX=0, lastY=0, lastZ=0;
            for (float step = 0; step < 8.0f; step += 0.05f) {
                lastX = floor(rayX); lastY = floor(rayY); lastZ = floor(rayZ);
                rayX += dirX * 0.05f; rayY += dirY * 0.05f; rayZ += dirZ * 0.05f;
                if (uMsg == WM_LBUTTONDOWN) {
                    for (int i = 0; i < MAX_DRONES; ++i) if (drones[i].active) {
                        if ((pow(drones[i].x - rayX, 2) + pow(drones[i].y - rayY, 2) + pow(drones[i].z - rayZ, 2)) < 1.0f) {
                            drones[i].active = false; score++; PlaySoundEffect(150, 150);
                            goto raycast_end;
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
                            worldData[prevIndex] = blockData; PlaySoundEffect(600, 50);
                        }
                    } else { // WM_MBUTTONDOWN
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
        } break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}