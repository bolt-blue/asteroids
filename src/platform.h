/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef PLATFORM_H
#define PLATFORM_H

/*
 * Here we define the required interface that every platform layer must provide
 */

#include "asteroids.h"
#include "po_window.h"

/* ========================================================================== */

typedef struct po_memory po_memory;
struct po_memory {
    size_t size;
    void *base;
};

/* ========================================================================== */

po_memory po_map_mem(size_t size);
void po_unmap_mem(po_memory *memory);
int po_get_input_state(struct po_window *window, game_input *input);
void po_render_to_screen(po_window *window);

#endif /* PLATFORM_H */
