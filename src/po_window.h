/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef PO_WINDOW_H
#define PO_WINDOW_H

#include <stdint.h>

/* ========================================================================== */

// These structs must be defined separately by each platform layer
typedef struct po_window po_window;
typedef struct po_surface po_surface;

typedef struct po_pixel po_pixel;
struct po_pixel {
    uint8_t b, g, r, a;
};

typedef enum po_key po_key;
enum po_key {
    PO_KEY_NONE,
    PO_KEY_Q,
};

/* ========================================================================== */

// The following function prototypes must be defined by each platform layer

struct po_window po_window_init(uint16_t width, uint16_t height);
void po_window_destroy(struct po_window *window);

enum po_key po_key_pressed(struct po_window *window);

void po_render_surface(po_window *window);

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

#endif /* PO_WINDOW_H */
