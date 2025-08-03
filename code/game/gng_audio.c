#include "gng_bool.h"
#include "gng_audio.h"

sound_man *soundMan;

void soundManInit(sound_man *sm, asset_man *assetMan) {
    soundMan = sm;
    soundMan->assetMan = assetMan;
}

void soundManPlaySound (char *key, b32 loop) {
    for (u32 i = 0; i < SOUND_MAN_MAX_PLAYING_SOUNDS; i++) {
        playing_sound *playingSound = &soundMan->playingSounds[i];
        if (!playingSound->active) {
            playingSound->active = true;
            playingSound->stopped = false;
            playingSound->loop = loop;
            sound_asset *soundAsset = getSound(soundMan->assetMan, key);
            playingSound->key = soundAsset->key; // use the pointer assigned to the sound asset, which is permanent
            playingSound->currentSampleIndex = 0;
            break;
        }
    }
}

b32 soundManIsPlaying (char *key) {
    for (u32 i = 0; i < SOUND_MAN_MAX_PLAYING_SOUNDS; i++) {
        playing_sound *playingSound = &soundMan->playingSounds[i];
        if (playingSound->key == key) {
            return playingSound->active && !playingSound->stopped;
        }
    }
    return false;
}

void soundManStopSound (char *key) {
    for (u32 i = 0; i < SOUND_MAN_MAX_PLAYING_SOUNDS; i++) {
        playing_sound *playingSound = &soundMan->playingSounds[i];
        if (playingSound->active && playingSound->key == key) {
            playingSound->stopped = true;
            playingSound->fadeT = 0.2f;
            break;
        }
    }
}
