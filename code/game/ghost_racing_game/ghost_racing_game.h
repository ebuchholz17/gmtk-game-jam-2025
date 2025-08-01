#ifndef GHOST_RACING_GAME_H
#define GHOST_RACING_GAME_H

#include "../gng_math.h"
#include "../gng_types.h"

typedef struct DebugCamera {
    vec3 pos;
    quat rotation;
    int lastPointerX;
    int lastPointerY;
} DebugCamera;

typedef struct GrGame {
    b32 isInitialized;
} GrGame;

#endif
