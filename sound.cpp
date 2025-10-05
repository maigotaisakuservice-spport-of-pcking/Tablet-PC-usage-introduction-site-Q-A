#include "globals.h"
#include "sound.h"
#include <windows.h>
#include <time.h>

static HANDLE musicThreadHandle;
static bool music_thread_running = false;

DWORD WINAPI PlaySoundThread(LPVOID lpParam) {
    int* params = (int*)lpParam;
    Beep(params[0], params[1]);
    delete[] params;
    return 0;
}

void PlaySoundEffect(int frequency, int duration) {
    int* params = new int[2];
    params[0] = frequency;
    params[1] = duration;
    CreateThread(NULL, 0, PlaySoundThread, params, 0, NULL);
}

DWORD WINAPI MusicLoop(LPVOID lpParam) {
    srand(time(NULL));
    const int scale[] = {261, 293, 329, 349, 392, 440, 493, 523}; // C Major scale

    while(music_thread_running) {
        if(musicEnabled) {
            int note = scale[rand() % 8];
            int duration = 150 + (rand() % 200);
            Beep(note, duration);
            Sleep(200 + (rand() % 300));
        } else {
            Sleep(500); // Sleep if music is disabled
        }
    }
    return 0;
}

void StartMusicThread() {
    if (!music_thread_running) {
        music_thread_running = true;
        musicThreadHandle = CreateThread(NULL, 0, MusicLoop, NULL, 0, NULL);
    }
}

void StopMusicThread() {
    if (music_thread_running) {
        music_thread_running = false;
        if (musicThreadHandle != NULL) {
            WaitForSingleObject(musicThreadHandle, 2000); // Wait for thread to finish
            CloseHandle(musicThreadHandle);
        }
    }
}