/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef PLATFORM_H
#define PLATFORM_H

/*
 * Here we define the required interface that every platform layer must provide
 */

#include "asteroids.h"
#include "po_window.h"

/* ========================================================================== */

int po_get_input_state(struct po_window *window, game_input *input);
void po_render_to_screen(po_window *window);

#endif /* PLATFORM_H */
