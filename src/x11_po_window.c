#include "po_window.h"

po_window window_init(size_t width, size_t height)
{
    po_window window = {0};

    // Connect using the DISPLAY environment variable
    window.connection = xcb_connect(NULL, NULL);

    // Get the first screen
    const xcb_setup_t *setup = xcb_get_setup(window.connection);
    window.screen = xcb_setup_roots_iterator(setup).data;

    uint16_t window_width  = width;
    uint16_t window_height = height;

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

    // Set up the actual window
    xcb_window_t window_id = xcb_generate_id(window.connection);
    xcb_create_window(window.connection,
            XCB_COPY_FROM_PARENT,                   /* depth (same as root) */
            window_id,                              /* window id            */
            window.screen->root,                    /* parent window        */
            0, 0,                                   /* x, y (ignored by WM) */
            window_width, window_height,            /* width, height        */
            10,                                     /* border_width         */
            XCB_WINDOW_CLASS_INPUT_OUTPUT,          /* class                */
            window.screen->root_visual,             /* visual               */
            masks, mask_values);                    /* masks                */

    // Draw the window
    xcb_map_window(window.connection, window_id);
    xcb_flush(window.connection);

    return window;
}

void window_destroy(po_window *window)
{
    xcb_disconnect(window->connection);
    window->connection = NULL;
    // setup is invalidated when the connection is freed
    // Ref: https://xcb.freedesktop.org/PublicApi/#index7h2
    window->setup = NULL;
    // can't find in the docs, but we presume the same is true for screen
    window->screen = NULL;
}
