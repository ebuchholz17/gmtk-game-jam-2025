#include "ghost_racing_game.h"

#include "../gng_assets.h"
#include "../gng_bool.h"
#include "../gng_math.h"
#include "../gng_memory.h"
#include "../gng_platform.h"
#include "../gng_sprites.h"
#include "../gng_util.h"
#include "../gng_virtual_input.h"
#include "../gng_3d.h"

#include "track_paths.h"

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

void startGame (mem_arena *tempMemory) {
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

    grGame->lapZoneIndex = 0;
    grGame->lap = 1;
    grGame->currentLapTime = 0.f;
    grGame->lapTimeIndex = 0;
    grGame->hasBestLapTime = false;

    grGame->ghostIndex = 0;
}


void initGrGame (GrGame *grg, mem_arena *memory, mem_arena *tempMemory) {
    grGame = grg;
    zeroMemory((u8 *)grGame, sizeof(GrGame));

    // initialization
    // load and set one-time stuff
    // o
    
    grGame->walls = Wall_listInit(memory, 100);
    for (i32 i = 0; i < ARRAY_COUNT(trackPaths); i += 2) {
        vec2 start = trackPaths[i];
        vec2 end = trackPaths[i+1];

        Wall wall = {
            .startPos = start,
            .endPos = end
        };
        Wall_listPush(&grGame->walls, wall);
    }

    grGame->ghostData = allocMemory(memory, sizeof(GhostData) * 50);
    
    // startGame function to init (or restart) game state
    startGame(tempMemory);

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
    result.brake = input->downArrow.down || vInput->rightButton.button.down;
    result.steerLeft = input->leftArrow.down || vInput->dPadLeft.button.down;
    result.steerRight = input->rightArrow.down || vInput->dPadRight.button.down;

    return result;
}

// TODO(ebuchholz): handle going backwards?
void handleLaps (void) {
    grGame->lapFrame++;

    vec2 carPixelPos = trackToPixelCoords(grGame->playerCar.trackPos);
    for (i32 i = 0; i < ARRAY_COUNT(lapZones); i++) {
        rect *lapZone = &lapZones[i];
        if (rectContainsPoint(*lapZone, carPixelPos.x, carPixelPos.y)) {
            i32 lastIndex = grGame->lapZoneIndex;
            grGame->lapZoneIndex = i;

            if (grGame->lapZoneIndex == 0 && lastIndex == ARRAY_COUNT(lapZones) - 1) {
                grGame->lap++;
                grGame->lapFrame = 0;
                grGame->ghostIndex++;
                if (grGame->ghostIndex > 50) {
                    grGame->ghostIndex = 50;
                }

                if (grGame->hasBestLapTime) {
                    if (grGame->currentLapTime < grGame->bestLapTime) {
                        grGame->bestLapTime = grGame->currentLapTime;
                    }
                }
                else {
                    grGame->hasBestLapTime = true;
                    grGame->bestLapTime = grGame->currentLapTime;
                }

                grGame->lapTimes[grGame->lapTimeIndex] = grGame->currentLapTime;
                grGame->lapTimeIndex = (grGame->lapTimeIndex + 1) % 3;
                grGame->currentLapTime = 0.f;
            }
        }
    }
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
    f32 acceleration = 11.0f;
    f32 maxSpeed = 180.0f;
    if (input->throttle) {
        grGame->playerCar.speed += 11.0f * dt;
    }
    else {
        grGame->playerCar.speed -= 5.0f * dt;
    }

    if (input->brake) {
        grGame->playerCar.speed -= 50.0f * dt;
    }

    if (grGame->playerCar.speed > maxSpeed) {
        grGame->playerCar.speed = maxSpeed;
    }
    if (grGame->playerCar.speed < 0.f) {
        grGame->playerCar.speed = 0.f;
    }

    grGame->playerCar.trackPos.x += grGame->playerCar.speed * grGame->playerCar.forward.x * dt;
    grGame->playerCar.trackPos.y += grGame->playerCar.speed * grGame->playerCar.forward.y * dt;

    Car *playerCar = &grGame->playerCar;

    f32 turnSpeed = 0.35f;
    f32 angle = arctangent2(playerCar->forward.y, playerCar->forward.x);
    angle /= (2.0f * PI);
    f32 wheelSpeed = 3.0f;
    if (input->steerLeft) {
        playerCar->wheel -= wheelSpeed * dt;
    }
    else if (input->steerRight) {
        playerCar->wheel += wheelSpeed * dt;
    }
    else {
        if (playerCar->wheel > 0.0f) {
            playerCar->wheel -= wheelSpeed * dt;
            if (playerCar->wheel < 0.0f) {
                playerCar->wheel = 0.0f;
            }
        }
        else if (playerCar->wheel < 0.0f) {
            playerCar->wheel += wheelSpeed * dt;
            if (playerCar->wheel > 0.0f) {
                playerCar->wheel = 0.0f;
            }
        }
    }

    f32 maxWheel = 1.0f;
    if (playerCar->speed > 45.0f) {
        maxWheel = 1.0f - ((playerCar->speed - 45.0f) / 90.0f) * 0.5f;
    }

    if (playerCar->wheel < -maxWheel) {
        playerCar->wheel = -maxWheel;
    }
    if (playerCar->wheel > maxWheel) {
        playerCar->wheel = maxWheel;
    }

    if (playerCar->wheel != 0.0f) {
        angle += playerCar->wheel * turnSpeed * dt;
        playerCar->forward.x = fastCos2PI(angle);
        playerCar->forward.y = fastSin2PI(angle);
        playerCar->forward = vec2Normalize(playerCar->forward);
    }
}

void applyGround (f32 dt) {
    vec2 carPixelPos = trackToPixelCoords((vec2){ grGame->playerCar.trackPos.x, grGame->playerCar.trackPos.y});
    int carPixelX = carPixelPos.x;
    int carPixelY = carPixelPos.y;
    if (carPixelX >= 0 && carPixelY >= 0 && carPixelX < 4096 && carPixelY < 4096) {
        texture_asset *track = getTexture(assetMan, "track");

        u8 *pixel = &track->pixels[carPixelY * track->width * 4 + carPixelX * 4];
        u8 r = pixel[0];
        u8 g = pixel[1];
        u8 b = pixel[2];
        b32 onTrack = false;
        if ((r == 28 && g == 41 && b == 50) ||
            (r == 255 && g == 255 && b == 255)) 
        {
            onTrack = true;
        }

        if (!onTrack) {
            grGame->playerCar.speed -= grGame->playerCar.speed * 1.f * dt;
        }
    }
}

vec2 closestPointOnWallToPoint (Wall *wall, vec2 point) {
    vec2 ab = vec2Subtract(wall->endPos, wall->startPos);
    f32 t = vec2Dot(vec2Subtract(point, wall->startPos), ab) / vec2Dot(ab, ab);
    if (t < 0.0f) { t = 0.0f; }
    if (t > 1.0f) { t = 1.0f; }
    vec2 result = vec2Add(wall->startPos, vec2ScalarMul(t, ab));
    return result;
}

void handleWalls (f32 dt) {
    f32 wallThickness = 15.f;
    vec2 carPixelCoords = trackToPixelCoords(grGame->playerCar.trackPos);
    for (i32 i = 0; i < grGame->walls.numValues; i++) {
        Wall *wall = &grGame->walls.values[i];

        vec2 closestPoint = closestPointOnWallToPoint(wall, carPixelCoords);
        vec2 disp = vec2Subtract(carPixelCoords, closestPoint);
        f32 dist = vec2Length(disp);
        if (dist < wallThickness) {
            vec2 dispNorm = vec2Normalize(disp);
            vec2 newPos = vec2Add(closestPoint, vec2ScalarMul(wallThickness + 0.01f, dispNorm));
            grGame->playerCar.trackPos = pixelToTrackCoords(newPos);
            // TODO(ebuchholz): cancel the portion of the velocity in the walls direction
            grGame->playerCar.speed -= 10.0f * dt;
        }
    }
}


void updateGrGame (GrGame *grg, game_input *input, virtual_input *vInput, f32 dt, plat_api platAPI, mem_arena *memory) {
    // check to restart game
    //debugCameraMovement(&grGame->debugCamera, input, dt);

    CarInput carInput = handleInput(input, vInput);
    steerCar(&carInput, dt);
    applyGround(dt);
    handleWalls(dt);
    handleLaps();

    grGame->debugCamera.pos = (vec3){ .x=grGame->playerCar.trackPos.x, .y = 10.0f, .z=grGame->playerCar.trackPos.y };
    grGame->debugCamera.pos = vec3Subtract(grGame->debugCamera.pos, vec3ScalarMul(5.0f, (vec3){ .x=grGame->playerCar.forward.x, .y=0.f, .z=grGame->playerCar.forward.y }));
    //vec3 trackPos3D = (vec3){.x=grGame->playerCar.trackPos.x, .y=0.f, .z=grGame->playerCar.trackPos.y};
    //vec3 forward3D = (vec3){.x=grGame->playerCar.forward.x, .y=0.f, .z=grGame->playerCar.forward.y};
    //vec3 lookAtPos = vec3Add(trackPos3D, vec3ScalarMul(25.0f, forward3D));
    //grGame->debugCamera.rotation = createLookAtQuaternion(grGame->debugCamera.pos.x, grGame->debugCamera.pos.y, grGame->debugCamera.pos.z,
    //                                                        lookAtPos.x, lookAtPos.y, lookAtPos.z);
    //                                                        o
    grGame->currentLapTime += dt;

    GhostData *currentData = &grGame->ghostData[grGame->ghostIndex];
    if (currentData->entryIndex < 3 * 60 * 60) {
        currentData->entries[currentData->entryIndex++] = (GhostDataEntry){
            .pos = grGame->playerCar.trackPos,
            .forward = grGame->playerCar.forward,
        };
    }

    for (i32 i = 0; i < (grGame->lap - 1) && i < 50; i++) {
        GhostRacer *racer = &grGame->ghostRacers[i];
        racer->ghostIndex = i;
        GhostData *data = &grGame->ghostData[racer->ghostIndex];
        racer->frame++;
        if (racer->frame >= data->entryIndex) {
            racer->frame = 0;
        }
    }
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

void drawTime (f32 timeF, f32 x, f32 y) {
    u32 time = (u32)timeF;
    u32 seconds = time % 60;
    f32 millisecondsF = timeF - (f32)time;
    u32 milliseconds = (u32)(millisecondsF * 100.f);
    u32 minutes = time / 60;
    char *timeString = tempStringFromI32((i32)minutes);
    timeString = tempStringAppend(timeString, ":");
    if (seconds < 10) {
        timeString = tempStringAppend(timeString, "0");
    }
    timeString = tempStringAppend(timeString, tempStringFromI32((i32)seconds));
    timeString = tempStringAppend(timeString, ".");
    if (milliseconds < 10) {
        timeString = tempStringAppend(timeString, "0");
    }
    timeString = tempStringAppend(timeString, tempStringFromI32((i32)milliseconds));

    spriteManAddText((sprite_text){
        .x = x,
        .y = y,
        .text = timeString,
        .fontKey = "font"
    });
}

void drawGrGame (GrGame *grg, plat_api platAPI) { 
        // TODO(ebuchholz): testing, remove
        basic3DMan->shouldDraw = true;
        //mat4x4 view = createViewMatrix(quat rotation, float x, float y, float z) ;
        mat4x4 view = createViewMatrix(grGame->debugCamera.rotation, 
                                       grGame->debugCamera.pos.x,
                                       grGame->debugCamera.pos.y,
                                       grGame->debugCamera.pos.z);

    vec3 trackPos3D = (vec3){.x=grGame->playerCar.trackPos.x, .y=0.f, .z=grGame->playerCar.trackPos.y};
    vec3 forward3D = (vec3){.x=grGame->playerCar.forward.x, .y=0.f, .z=grGame->playerCar.forward.y};
    vec3 lookAtPos = vec3Add(trackPos3D, vec3ScalarMul(12.0f, forward3D));
        view = createLookAtMatrix(grGame->debugCamera.pos.x, grGame->debugCamera.pos.y, grGame->debugCamera.pos.z,
                                                            lookAtPos.x, lookAtPos.y, lookAtPos.z,
                                                                0.0f, 1.0f, 0.f);

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
    mat4x4 proj = createPerspectiveMatrix(nearViewDist, farViewDist, viewRatio, 0.8f);

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

    drawBillBoard(grGame->playerCar.trackPos, "car_0", 50.0f);


    for (i32 i = 0; i < (grGame->lap - 1) && i < 50; i++) {
        GhostRacer *racer = &grGame->ghostRacers[i];
        GhostData *data = &grGame->ghostData[racer->ghostIndex];
        GhostDataEntry *entry = &data->entries[racer->frame];
        drawBillBoard(entry->pos, "car_0", 50.0f);
    }


    {
        sprite s = defaultSprite();
        s.pos = (vec2){ .x=56.f, .y=201.0f};
        s.atlasKey = "game_atlas";
        s.frameKey = "speedometer_base";
        s.anchor = (vec2){ .x=0.5f, .y=1.0f };
        spriteManAddSprite(s);
    }

    f32 speedRotatePercent = grGame->playerCar.speed / 257.0f;
    f32 rotationRange = 0.65;
    f32 baseRotation = 0.425;

    {
        sprite s = defaultSprite();
        s.pos = (vec2){ .x=56.f, .y=177.0f};
        s.atlasKey = "game_atlas";
        s.frameKey = "needle";
        s.anchor = (vec2){ .x=0.5f, .y=0.5f };
        s.rotation = baseRotation + rotationRange * speedRotatePercent;
        spriteManAddSprite(s);
    }

    {
        char *text = tempStringAppend("Lap ", tempStringFromI32(grGame->lap));
        sprite_text labelText = {
            .text = text,
            .fontKey = "font",
            .x = 244.0f,
            .y = 188.0f
        };
        //centerText(&labelText);
        spriteManAddText(labelText);
    }

    drawTime(grGame->currentLapTime, 294.0f, 188.0f);

    i32 lapIndex = grGame->lapTimeIndex - 1;
    f32 timeY = 188.0f;
    for (i32 i = 0; i < (grGame->lap - 1) && i < 3; i++) {
        if (lapIndex < 0) { lapIndex += 3; }
        f32 time = grGame->lapTimes[lapIndex];
        timeY -= 12.0f;
        drawTime(time, 294.0f, timeY);
        lapIndex = lapIndex - 1;
    }

    timeY -= 12.0f;
    if (grGame->hasBestLapTime) {
        {
            char *text = "Best";
            sprite_text labelText = {
                .text = text,
                .fontKey = "font",
                .x = 244.0f,
                .y = timeY
            };
            //centerText(&labelText);
            spriteManAddText(labelText);
        }
        drawTime(grGame->bestLapTime, 294.0f, timeY);
    }
}
