#include "sound.h"
#include <windows.h>

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