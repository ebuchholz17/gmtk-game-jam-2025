#ifndef GNG_AUDIO_H
#define GNG_AUDIO_H

#include "gng_types.h"
#include "gng_assets.h"

typedef struct playing_sound {
    b32 active;
    char *key;
    u32 currentSampleIndex;
    b32 stopped;
    b32 loop;
    f32 fadeT;
} playing_sound;

#define SOUND_MAN_MAX_PLAYING_SOUNDS 30
typedef struct sound_man {
    playing_sound playingSounds[SOUND_MAN_MAX_PLAYING_SOUNDS];
    asset_man *assetMan;
} sound_man;

void soundManPlaySound (char *key, b32 loop);
b32 soundManIsPlaying (char *key);
void soundManStopSound (char *key);

#endif
