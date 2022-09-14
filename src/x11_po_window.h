/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef X11_PO_WINDOW_H
#define X11_PO_WINDOW_H

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

struct po_window {
    xcb_connection_t *connection;
    // TODO: Do we really need to store the following?
    const xcb_setup_t *setup;
    const xcb_screen_t *screen;

    xcb_key_symbols_t *keysyms;
};

#endif /* X11_PO_WINDOW_H */
