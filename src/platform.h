/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef PLATFORM_H
#define PLATFORM_H

/*
 * Here we define the required interface that every platform layer must provide
 */

#include "po_types.h"
#include "po_window.h"

/* ========================================================================== */

#define PULSE THIRTY_FPS

/* ========================================================================== */

typedef struct po_pixel po_pixel;
struct po_pixel {
    uint8_t b, g, r, a;
};

typedef struct offscreen_draw_buffer offscreen_draw_buffer;
struct offscreen_draw_buffer {
    size_t width;
    size_t height;
    po_pixel *data;
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

typedef struct po_memory po_memory;
struct po_memory {
    size_t size;
    void *base;
};

typedef struct game_memory game_memory;
struct game_memory {
    struct game_state *state;

    // TODO: Platform API function pointers
    int8_t *base;
};

/* ========================================================================== */

po_memory po_map_mem(size_t size);
void po_unmap_mem(po_memory *memory);
int po_get_input_state(struct po_window *window, game_input *input);
void po_render_to_screen(po_window *window);

#define GAME_INIT(name) int name(game_memory *memory,                          \
        size_t persistent_storage_size, size_t temporary_storage_size,         \
        const offscreen_draw_buffer *buffer)
typedef GAME_INIT(GameInit);
GAME_INIT(game_init_stub)
{
    return 0;
}

#define GAME_UPDATE_AND_RENDER(name) int name(game_memory *memory,             \
        game_input *input, offscreen_draw_buffer *buffer)
typedef GAME_UPDATE_AND_RENDER(GameUpdateAndRender);
GAME_UPDATE_AND_RENDER(game_update_and_render_stub)
{
    return 0;
}

#endif /* PLATFORM_H */
