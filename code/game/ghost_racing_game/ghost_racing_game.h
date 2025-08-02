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
    f32 speed;
    f32 wheel;
} Car;

typedef struct CarInput {
    b32 throttle;
    b32 brake;
    b32 steerLeft;
    b32 steerRight;
} CarInput;

typedef struct Wall {
    vec2 startPos;
    vec2 endPos;
} Wall;

#define LIST_TYPE Wall
#include "../list.h"

typedef struct GrGame {
    DebugCamera debugCamera;
    b32 isInitialized;

    Car playerCar;
    Wall_list walls;

    i32 lapZoneIndex;
    i32 lap;
} GrGame;

#endif
