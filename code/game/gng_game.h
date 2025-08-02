#ifndef GNG_GAME_H
#define GNG_GAME_H

#include "gng_bool.h"
#include "gng_types.h"
#include "gng_platform.h"
#include "gng_assets.h"
#include "gng_sprites.h"
#include "gng_3d.h"
#include "gng_audio.h"
#include "gng_virtual_input.h"

#include "hitbox/hitbox.h"
#include "ghost_racing_game/ghost_racing_game.h"

typedef struct spinning_triangle {
    f32 x;
    f32 y;
    f32 rotation;
} spinning_triangle;

typedef struct gng_game_state {
    b32 initialized;

    mem_arena memory;
    asset_man assetMan;
    sprite_man spriteMan;
    sound_man soundMan;
    basic_3d_man basic3DMan;

    f32 t;
    virtual_input vInput;

    GrGame grGame;

    f32 soundT;

} gng_game_state;

#endif
