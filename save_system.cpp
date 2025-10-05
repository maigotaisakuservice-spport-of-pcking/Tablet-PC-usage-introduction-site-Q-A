#include "globals.h"
#include "save_system.h"
#include "utils.h"
#include <windows.h>

void ScanForWorlds() {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile("*.world", &findFileData);

    world_count = 0;
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (world_count < 5) {
                tiny_strcpy(world_names[world_count], findFileData.cFileName);
                world_count++;
            }
        } while (FindNextFile(hFind, &findFileData) != 0 && world_count < 5);
        FindClose(hFind);
    }
}

// Simple header to store player data along with world data
struct WorldFileHeader {
    float playerX, playerY, playerZ;
    float playerYaw, playerPitch;
    int score_val;
    int playerHP_val;
    GameMode gameMode_val;
    float gameTime_val;
};

bool SaveWorld(const char* worldName) {
    HANDLE hFile = CreateFile(worldName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    WorldFileHeader header;
    header.playerX = camX;
    header.playerY = camY;
    header.playerZ = camZ;
    header.playerYaw = camYaw;
    header.playerPitch = camPitch;
    header.score_val = score;
    header.playerHP_val = playerHP;
    header.gameMode_val = gameMode;
    header.gameTime_val = gameTime;

    DWORD bytesWritten;
    WriteFile(hFile, &header, sizeof(header), &bytesWritten, NULL);
    WriteFile(hFile, worldData, (size_t)WORLD_WIDTH * WORLD_HEIGHT * WORLD_DEPTH, &bytesWritten, NULL);

    CloseHandle(hFile);
    return true;
}

bool LoadWorld(const char* worldName) {
    HANDLE hFile = CreateFile(worldName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    WorldFileHeader header;
    DWORD bytesRead;
    ReadFile(hFile, &header, sizeof(header), &bytesRead, NULL);
    if(bytesRead != sizeof(header)) {
        CloseHandle(hFile);
        return false;
    }
    ReadFile(hFile, worldData, (size_t)WORLD_WIDTH * WORLD_HEIGHT * WORLD_DEPTH, &bytesRead, NULL);
    if(bytesRead != (size_t)WORLD_WIDTH * WORLD_HEIGHT * WORLD_DEPTH){
        CloseHandle(hFile);
        return false;
    }


    CloseHandle(hFile);

    camX = header.playerX;
    camY = header.playerY;
    camZ = header.playerZ;
    camYaw = header.playerYaw;
    camPitch = header.playerPitch;
    score = header.score_val;
    playerHP = header.playerHP_val;
    gameMode = header.gameMode_val;
    gameTime = header.gameTime_val;

    // Reset transient states
    for(int i=0; i<MAX_DRONES; ++i) drones[i].active = false;
    for(int i=0; i<MAX_ZOMBIES; ++i) zombies[i].active = false;
    for(int i=0; i<MAX_PIGS; ++i) pigs[i].active = false;
    for(int i=0; i<MAX_COWS; ++i) cows[i].active = false;
    boss.active = false;

    return true;
}