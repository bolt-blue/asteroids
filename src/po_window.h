/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef PO_WINDOW_H
#define PO_WINDOW_H

/*
 * TODO: Wayland support
 * For linux, we will build either for X11 or Wayland - X11 by default
 * To compile using Wayland, set USE_WAYLAND during your chosen build method
 * For unix in general we stick with X11
 */

/* ========================================================================== */

// Platform-dependent includes
#ifdef __unix__
#include "unix_po_window.h"

#elif __APPLE__
    // TODO: Apple support (OSX and iOS)

#elif _WIN32
    // TODO: Windows support

#elif __ANDROID__
    // TODO: Android support

#else
#error Platform not supported
#endif // Platform-dependent

/* ========================================================================== */

// TODO: Use our own size and type definitions ?
#include <stddef.h>
#include <stdint.h>

#include "po_arena.h"

/* ========================================================================== */

// This struct must be defined separately by each platform layer
// See the above platform-dependent includes

typedef struct po_window po_window;

/* ========================================================================== */

typedef struct po_pixel po_pixel;
struct po_pixel {
    uint8_t b, g, r, a;
};

/* ========================================================================== */

struct po_window po_window_init(uint16_t width, uint16_t height, po_arena *arena);
void po_window_destroy(struct po_window *window);

#endif /* PO_WINDOW_H */
