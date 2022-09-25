/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef X11_PO_WINDOW_H
#define X11_PO_WINDOW_H

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include "po_types.h"

struct po_window {
    uint16_t width;
    uint16_t height;

    xcb_window_t win_id;
    xcb_pixmap_t pm_id;
    xcb_gcontext_t gc_id;

    // TODO: Only store the essentials here
    xcb_connection_t *connection;
    const xcb_setup_t *setup;
    const xcb_screen_t *screen;
    xcb_key_symbols_t *keysyms;

    const struct po_pixel *buffer;
};

#endif /* X11_PO_WINDOW_H */
