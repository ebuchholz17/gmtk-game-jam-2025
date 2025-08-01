#include "ghost_racing_game.h"

#include "../gng_bool.h"
#include "../gng_memory.h"
#include "../gng_platform.h"
#include "../gng_util.h"
#include "../gng_virtual_input.h"

GrGame *grGame;

void initGrGame (GrGame *grg, mem_arena *memory) {
    grGame = grg;
    zeroMemory((u8 *)grGame, sizeof(GrGame));

    // initialization
    // load and set one-time stuff
    // startGame function to init (or restart) game state

    grGame->isInitialized = true;
}

void debugCameraMovement (DebugCamera *debugCamera, game_input *input, f32 dt) {
    const float CAMERA_SPEED = 10.0f;
    const float CAMERA_TURN_SPEED = 1.0f;

    // Position
    vec3 moveVector = {};
    if (input->upArrow.down) {
        moveVector.z -= CAMERA_SPEED * dt;
    }
    if (input->downArrow.down) {
        moveVector.z += CAMERA_SPEED * dt;
    }
    if (input->leftArrow.down) {
        moveVector.x -= CAMERA_SPEED * dt;
    }
    if (input->rightArrow.down) {
        moveVector.x += CAMERA_SPEED * dt;
    }

    // Rotation
    if (input->pointerJustDown) {
        debugCamera->lastPointerX = input->pointerX;
        debugCamera->lastPointerY = input->pointerY;
    }
    if (input->pointerDown) {
        int pointerDX = input->pointerX - debugCamera->lastPointerX;
        int pointerDY = input->pointerY - debugCamera->lastPointerY;

        float yaw = (float)pointerDX * 0.25f;
        float pitch = (float)pointerDY * 0.25f;

        vec3 axis = (vec3){.x=0.f, .y=1.f, .z=0.f};
        f32 angle = -yaw * dt;
        debugCamera->rotation = quatMul(quaternionFromAxisAngle(axis, angle), debugCamera->rotation);
        debugCamera->rotation = quatMul(debugCamera->rotation, quaternionFromAxisAngle((vec3){.x=1.f, .y=0.f, .z=0.f}, -pitch * dt));
        debugCamera->lastPointerX = input->pointerX;
        debugCamera->lastPointerY = input->pointerY;
    }
    // Move in the direction of the current rotation
    moveVector = rotateVectorByQuaternion(moveVector, debugCamera->rotation);
    debugCamera->pos += moveVector;
}

void updateGrGame (GrGame *grg, game_input *input, virtual_input *vInput, f32 dt, plat_api platAPI, mem_arena *memory) {
    // check to restart game
}
