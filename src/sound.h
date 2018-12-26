#ifndef SOUND_H
#define SOUND_H
typedef enum  {
    PREAMBLE,
    MUSIC_DETECTED
} state_t;

void soundSetup();
void soundThread();

String getLog();

#endif
