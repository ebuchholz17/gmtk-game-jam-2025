#ifndef GNG_3D_H
#define GNG_3D_H

#include "gng_math.h"

// Draws ONE 3D quad with camera, proj, texture, etc.
typedef struct basic_3d_man {
    mat4x4 model;
    mat4x4 view;
    mat4x4 proj;
    char *textureKey;
    b32 shouldDraw;
} basic_3d_man;

void basic3DManInit (basic_3d_man *bm);

#endif
