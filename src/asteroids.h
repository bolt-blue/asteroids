/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include <stddef.h>
#include <stdint.h>

#include "po_arena.h"
#include "po_window.h"

/* ========================================================================== */

#define PULSE THIRTY_FPS

/* ========================================================================== */

typedef struct game_memory game_memory;
struct game_memory {
    po_arena persistent_memory;
    po_arena temporary_memory;
};

typedef struct offscreen_draw_buffer offscreen_draw_buffer;
struct offscreen_draw_buffer {
    size_t width;
    size_t height;
    po_pixel *data;
};

typedef struct input_state input_state;
struct input_state {
    uint32_t is_down;
};

typedef struct game_input game_input;
struct game_input {
    input_state thrust;
    input_state left;
    input_state right;
    input_state hyper;
    input_state fire;
    uint32_t quit;
};

// The option to pass whole context to game_update_and_render
#if 0
typedef struct po_context po_context;
struct po_context {
    game_memory memory;
    game_input input;
    offscreen_draw_buffer *buffer;
};
#endif

/* ========================================================================== */

int game_update_and_render(game_memory *memory, game_input *input, offscreen_draw_buffer *buffer);

#endif /* ASTEROIDS_H */
