#include "po_window.h"

/* ========================================================================== */

// TODO: Error handling

/*
 * Create and initialise a window
 *
 * In this context and window is mainly a means to having a plain old drawing
 * surface.
 *
 */
po_window
po_window_init(uint16_t width, uint16_t height)
{
    po_window window = {0};

    // Connect using the DISPLAY environment variable
    window.connection = xcb_connect(NULL, NULL);

    // Get the first screen
    const xcb_setup_t *setup = xcb_get_setup(window.connection);
    window.screen = xcb_setup_roots_iterator(setup).data;

    window.width  = width;
    window.height = height;

    // Define required attributes
    // WARNING: The order of the values needs to match the order of their
    // respective masks, found in the definition of `xcb_cw_t` in xproto.h
    // NOTE: For xcb, masks and values appear to be consistently u32, but
    // at time of writing, the prototype for xcb_create_window takes a void *
    // for the value_list, and expected values are declared as enum's
    uint32_t masks = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t mask_values[] =
    {
        0x00333333,     /* dark grey 0x??RRGGBB */
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE,
    };

    // Store keyboard symbols
    // TODO: This may need to be strategically run more than once, as
    // technically the keyboard layout could change during runtime.
    // This is valid if I am correctly understanding what I've been reading in
    // the documentation.
    window.keysyms = xcb_key_symbols_alloc(window.connection);

    // Create window
    window.win_id = xcb_generate_id(window.connection);
    xcb_create_window(window.connection,
            XCB_COPY_FROM_PARENT,                   /* depth (same as root) */
            window.win_id,                          /* window id            */
            window.screen->root,                    /* parent window        */
            0, 0,                                   /* x, y (ignored by WM) */
            window.width, window.height,            /* width, height        */
            10,                                     /* border_width         */
            XCB_WINDOW_CLASS_INPUT_OUTPUT,          /* class                */
            window.screen->root_visual,             /* visual               */
            masks, mask_values);                    /* masks                */

    // Create surface (pixmap)
    window.pm_id = xcb_generate_id(window.connection),

    // NOTE: Currently we're blitting directly to the window
    // so the pixmap is going totally unused?
    // See TODO in draw_surface()
    xcb_create_pixmap(window.connection, window.screen->root_depth,
            window.pm_id, window.win_id,
            window.width, window.height);

    // Create graphics context
    window.gc_id = xcb_generate_id(window.connection);
    xcb_create_gc(window.connection, window.gc_id, window.pm_id, 0, NULL);

    // Draw the window
    xcb_map_window(window.connection, window.win_id);
    xcb_flush(window.connection);

    return window;
}

void
po_window_destroy(po_window *window)
{
    xcb_disconnect(window->connection);
    xcb_key_symbols_free(window->keysyms);

    // NOTE: setup is invalidated when the connection is freed
    // Ref: https://xcb.freedesktop.org/PublicApi/#index7h2
    // can't find in the docs, but we presume the same is true for screen

    // All other allocations are internal and released separately as part of
    // the memory for the entire game
    *window = (po_window){0};
}
