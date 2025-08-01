#ifndef GNG_3D_H
#define GNG_3D_H

#include "gng_math.h"

// Draws ONE 3D quad with camera, proj, texture, etc.
typedef struct basic_3d_man {
    mat4x4 modelMatrix;
    mat4x4 viewMatrix;
    mat4x4 projMatrix;
    char *textureKey;
    b32 shouldDraw;
} basic_3d_man;

#endif
