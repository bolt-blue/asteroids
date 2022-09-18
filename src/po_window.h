/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef PO_WINDOW_H
#define PO_WINDOW_H

// TODO: Use our own size and type definitions ?
#include <stddef.h>
#include <stdint.h>

/* ========================================================================== */

/*
 *
 * TODO: Keep this comment up to date
 * For linux, we will build for both X11 and Wayland (eventually)
 * We start with and default to X11
 * If you want to compile using Wayland, set USE_WAYLAND during your chosen
 * build method
 *
 */

// Here we conditionally include the necessary platform code

#ifdef __linux__
#include "linux_po_window.h"
#elif __APPLE__
    // TODO: Apple support (OSX and iOS)
#elif _WIN32
    // TODO: Windows support
#elif __ANDROID__
    // TODO: Android support
#else
#error Platform not supported
#endif // Platform

/* ========================================================================== */

// These structs must be defined separately by each platform layer

typedef struct po_window po_window;
typedef struct po_surface po_surface;

/* ========================================================================== */

typedef struct po_arena po_arena;
struct po_arena {
    uint32_t capacity;
    uint32_t top;
    uint8_t *data;
};

typedef struct po_memory po_memory;
struct po_memory {
    po_arena persistent_memory;
    po_arena temporary_memory;
};

typedef struct po_context po_context;
struct po_context {
    po_window *window;
    po_memory game_memory;
};

typedef struct po_pixel po_pixel;
struct po_pixel {
    uint8_t b, g, r, a;
};

typedef enum po_key po_key;
enum po_key {
    PO_KEY_NONE,
    PO_KEY_Q,
    PO_KEY_W,
    PO_KEY_A,
    PO_KEY_S,
    PO_KEY_D,
};

typedef struct input_state po_input_state;
struct input_state {
    uint32_t is_down;
};

typedef struct game_input game_input;
struct game_input {
    po_input_state up;
    po_input_state down;
    po_input_state left;
    po_input_state right;
};

/* ========================================================================== */

// The following function prototypes must be defined by each platform layer

struct po_window po_window_init(uint16_t width, uint16_t height, po_arena *arena);
void po_window_destroy(struct po_window *window);

int po_get_input_state(struct po_window *window, game_input *input);

void po_render_surface(po_window *window);

po_arena po_arena_create(size_t size);
void po_arena_destroy(po_arena *arena);
void *po_arena_push(size_t size, po_arena *arena);
void po_arena_clear(po_arena *arena);

#endif /* PO_WINDOW_H */
