#include "ghost_racing_game.h"

#include "../gng_bool.h"
#include "../gng_math.h"
#include "../gng_memory.h"
#include "../gng_platform.h"
#include "../gng_sprites.h"
#include "../gng_util.h"
#include "../gng_virtual_input.h"
#include "../gng_3d.h"

// https://opensource.apple.com/source/Libm/Libm-315/Source/Intel/atan.c
#include "atan.c"

extern basic_3d_man *basic3DMan;

GrGame *grGame;

vec2 pixelToTrackCoords (vec2 pixelCoords) {
    return (vec2){ 
        .x = pixelCoords.x / 4096.f * 1000.f,
        .y = pixelCoords.y / 4096.f * 1000.f,
    };
}

vec2 trackToPixelCoords (vec2 trackCoords) {
    return (vec2){ 
        .x = trackCoords.x / 1000.f * 4096.f,
        .y = trackCoords.y / 1000.f * 4096.f,
    };
}

void startGame (void) {
    DebugCamera *debugCamera = &grGame->debugCamera;
    vec2 startPos = pixelToTrackCoords((vec2){ .x = 489.f, .y=2884.f });
    debugCamera->pos = (vec3){
        .x = startPos.x,
        .y = 3.0f,
        .z = startPos.y,
    };
    debugCamera->rotation = quaternionFromAxisAngle((vec3){0.0f, 1.0f, 0.0f}, 0.0f);
    //debugCamera->rotation = 
    //    quaternionFromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), 40.0f * (PI / 180.0f)) *
    //    quaternionFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), -33.0f * (PI / 180.0f));
    debugCamera->lastPointerX = 0;
    debugCamera->lastPointerY = 0;

    Car *playerCar = &grGame->playerCar;
    playerCar->trackPos = pixelToTrackCoords((vec2){ .x=500.f, .y = 3245.f });
    playerCar->forward = (vec2){.x=0, .y=-1.f};
}


void initGrGame (GrGame *grg, mem_arena *memory) {
    grGame = grg;
    zeroMemory((u8 *)grGame, sizeof(GrGame));

    // initialization
    // load and set one-time stuff
    // startGame function to init (or restart) game state
    startGame();


    grGame->isInitialized = true;
}

void debugCameraMovement (DebugCamera *debugCamera, game_input *input, f32 dt) {
    const float CAMERA_SPEED = 180.0f;
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
    debugCamera->pos = vec3Add(debugCamera->pos, moveVector);
}

CarInput handleInput (game_input *input, virtual_input *vInput) {
    CarInput result;

    result.throttle = input->upArrow.down || vInput->bottomButton.button.down;
    result.steerLeft = input->leftArrow.down || vInput->dPadLeft.button.down;
    result.steerRight = input->rightArrow.down || vInput->dPadRight.button.down;

    return result;
}

f32 arctangent2(f32 y, f32 x) {
    if (x > 0.0f) {
        return arctangent(y / x);
    }
    else if (x < 0.0f && y >= 0.0f) {
        return arctangent(y / x) + PI;
    }
    else if (x < 0.0f && y < 0.0f) {
        return arctangent(y / x) - PI;
    }
    else if (x == 0.0f && y > 0.0f) {
        return PI / 2.0f;
    }
    else if (x == 0.0f && y < 0.0f) {
        return -PI / 2.0f;
    }
    else {
        return 0.0f;
    }
}

void steerCar (CarInput *input, f32 dt) {
    f32 speed = 100.0f;
    if (input->throttle) {
        grGame->playerCar.trackPos.x += speed * grGame->playerCar.forward.x * dt;
        grGame->playerCar.trackPos.y += speed * grGame->playerCar.forward.y * dt;
    }

    f32 turnSpeed = 0.25f;
    f32 angle = arctangent2(grGame->playerCar.forward.y, grGame->playerCar.forward.x);
    angle /= (2.0f * PI);
    if (input->steerLeft) {
        angle -= turnSpeed * dt;
        grGame->playerCar.forward.x = fastCos2PI(angle);
        grGame->playerCar.forward.y = fastSin2PI(angle);
        grGame->playerCar.forward = vec2Normalize(grGame->playerCar.forward);
    }
    if (input->steerRight) {
        angle += turnSpeed * dt;
        grGame->playerCar.forward.x = fastCos2PI(angle);
        grGame->playerCar.forward.y = fastSin2PI(angle);
        grGame->playerCar.forward = vec2Normalize(grGame->playerCar.forward);
    }
}

void updateGrGame (GrGame *grg, game_input *input, virtual_input *vInput, f32 dt, plat_api platAPI, mem_arena *memory) {
    // check to restart game
    //debugCameraMovement(&grGame->debugCamera, input, dt);

    CarInput carInput = handleInput(input, vInput);
    steerCar(&carInput, dt);

    grGame->debugCamera.pos = (vec3){ .x=grGame->playerCar.trackPos.x, .y = 15.0f, .z=grGame->playerCar.trackPos.y };
    grGame->debugCamera.pos = vec3Subtract(grGame->debugCamera.pos, vec3ScalarMul(15.0f, (vec3){ .x=grGame->playerCar.forward.x, .y=0.f, .z=grGame->playerCar.forward.y }));
    vec3 trackPos3D = (vec3){.x=grGame->playerCar.trackPos.x, .y=0.f, .z=grGame->playerCar.trackPos.y};
    vec3 forward3D = (vec3){.x=grGame->playerCar.forward.x, .y=0.f, .z=grGame->playerCar.forward.y};
    vec3 lookAtPos = vec3Add(trackPos3D, vec3ScalarMul(25.0f, forward3D));
    grGame->debugCamera.rotation = createLookAtQuaternion(grGame->debugCamera.pos.x, grGame->debugCamera.pos.y, grGame->debugCamera.pos.z,
                                                            lookAtPos.x, lookAtPos.y, lookAtPos.z);
}

void drawBillBoard (vec2 trackPos, char *frameKey, f32 scale) {
    mat4x4 view = basic3DMan->view;
    mat4x4 proj = basic3DMan->proj;
    mat4x4 camMatrix = mat4x4MatrixMul(proj, view);
    f32 w = 1.0f;
    vec3 screenPos = transformPoint(camMatrix, (vec3){ .x=trackPos.x, .y=0.0f, .z=trackPos.y }, &w);
    screenPos.x /= w;
    screenPos.y /= w;
    screenPos.z /= w;

    f32 z = (1.0f - screenPos.z);
    if (z >= 0) {
        sprite s = defaultSprite();
        s.pos.x = (screenPos.x + 1.0f) / 2.0f * 356.f;
        s.pos.y = (1.f - ((screenPos.y + 1.0f) / 2.0f)) * 200.f;
        s.scale = z * scale;
        s.atlasKey = "game_atlas";
        s.frameKey = frameKey;
        s.anchor = (vec2){ .x = 0.5f, .y = 1.0f };
        spriteManAddSprite(s);
    }
}

void drawGrGame (GrGame *grg, plat_api platAPI) { 
        // TODO(ebuchholz): testing, remove
        basic3DMan->shouldDraw = true;
        //mat4x4 view = createViewMatrix(quat rotation, float x, float y, float z) ;
        mat4x4 view = createViewMatrix(grGame->debugCamera.rotation, 
                                       grGame->debugCamera.pos.x,
                                       grGame->debugCamera.pos.y,
                                       grGame->debugCamera.pos.z);

        // NOTE(ebuchholz): copy-pasted from gng_game
        f32 gameWidth = 356.0f;
        f32 gameHeight = 200.0f;

        float nearViewDist = 0.1f;
        float farViewDist = 1000.0f;
        float hFOV = 80.0f;
        float viewRatio = gameWidth / gameHeight;

       // float fovy = 2.0f * atanf(tanf(fov * (PI / 180.0f) / 2.0f) * (1.0f / aspectRatio));
       // float f = 1.0f / <tanf(fovy / 2.0f)>; <-- 4th param
       // float nf = 1.0f / (farPlane - nearPlane);
       
        // 1.234
        mat4x4 proj = createPerspectiveMatrix(nearViewDist, farViewDist, viewRatio, 0.5f);

        basic3DMan->view = view;
        basic3DMan->proj = proj;
        basic3DMan->textureKey = "track";

        mat4x4 model = identityMatrix4x4();
        mat4x4 scale = scaleMatrix4x4(500.f);
        mat4x4 rot90 = rotationMatrixFromAxisAngle((vec3){1.f, 0.f, 0.f}, -0.25);
        mat4x4 trans = translationMatrix(500.0f, 0.0f, 500.0f);
        model = mat4x4MatrixMul(scale, model);
        model = mat4x4MatrixMul(rot90, model);
        model = mat4x4MatrixMul(trans, model);

        basic3DMan->model = model;

        drawBillBoard(grGame->playerCar.trackPos, "car_0", 100.0f);
}
