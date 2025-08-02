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

typedef struct Car {
    vec2 trackPos;
    vec2 forward;
} Car;

typedef struct CarInput {
    b32 throttle;
    b32 brake;
    b32 steerLeft;
    b32 steerRight;
} CarInput;

typedef struct GrGame {
    DebugCamera debugCamera;
    b32 isInitialized;

    Car playerCar;
} GrGame;

#endif
