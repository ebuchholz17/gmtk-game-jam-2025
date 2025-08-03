#ifndef GHOST_RACING_GAME_H
#define GHOST_RACING_GAME_H

#include "../gng_math.h"
#include "../gng_sprites.h"
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
    b32 action;
} CarInput;

typedef struct Wall {
    vec2 startPos;
    vec2 endPos;
} Wall;

#define LIST_TYPE Wall
#include "../list.h"

typedef struct GhostDataEntry {
    vec2 pos;
    vec2 forward;
} GhostDataEntry;

typedef struct GhostData {
    GhostDataEntry entries[3 * 60 * 60];
    i32 entryIndex;
} GhostData;

typedef struct GhostRacer {
    i32 ghostIndex;
    i32 frame;
    b32 inRange;
} GhostRacer;

#define LIST_TYPE GhostData
#include "../list.h"

typedef enum {
    INPUT_SOURCE_KEYBOARD,
    INPUT_SOURCE_GAMEPAD,
    INPUT_SOURCE_VIRTUAL
} InputSource;

typedef enum {
    GR_GAME_STATE_TITLE,
    GR_GAME_STATE_TRACK_SELECT,
    GR_GAME_STATE_MAIN,
    GR_GAME_STATE_WIN,
} GrGameState;

typedef struct Billboard {
    sprite s;
    f32 z;
} Billboard;

#define LIST_TYPE Billboard
#include "../list.h"

typedef struct GrGame {
    DebugCamera debugCamera;
    b32 isInitialized;

    Car playerCar;
    Wall_list walls;

    i32 lapZoneIndex;
    i32 lap;

    f32 currentLapTime;
    f32 lapTimes[3];
    i32 lapTimeIndex;
    f32 bestLapTime;
    b32 hasBestLapTime;

    GhostData *ghostData;
    GhostRacer ghostRacers[50];
    i32 ghostIndex;
    i32 lapFrame;

    InputSource inputSource;
    b32 boosting;
    f32 boostCooldown;
    f32 boostDuration;
    f32 accelTimer;

    GrGameState gameState;
    f32 targetTime;
    b32 wasThrottle;
    b32 screeching;
    i32 track;

    i32 selectedOption;
} GrGame;

#endif
