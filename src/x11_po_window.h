/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef X11_PO_WINDOW_H
#define X11_PO_WINDOW_H

#include <stdint.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

struct po_surface {
    xcb_pixmap_t id;
    xcb_gcontext_t gc;
    size_t width;
    size_t height;
    struct po_pixel *data;
};

struct po_window {
    uint16_t width;
    uint16_t height;

    xcb_window_t id;

    xcb_connection_t *connection;
    // TODO: Do we really need to store setup?
    const xcb_setup_t *setup;
    const xcb_screen_t *screen;

    xcb_key_symbols_t *keysyms;

    struct po_surface *surface;
};

#endif /* X11_PO_WINDOW_H */
