/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef X11_PO_WINDOW_H
#define X11_PO_WINDOW_H

#include <xcb/xcb.h>

struct po_window {
    xcb_connection_t *connection;
    // TODO: Do we really need to store the following?
    const xcb_setup_t *setup;
    const xcb_screen_t *screen;
};

po_window window_init(size_t width, size_t height);
void window_destroy(po_window *window);

#endif /* X11_PO_WINDOW_H */
