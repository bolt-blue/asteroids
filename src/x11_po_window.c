#include <stdlib.h>  // free (seems a bit exessive, but oh well)

#include "po_window.h"

// TODO: Error handling

// Mask for the common key modifiers, ignoring those we're generally not
// interested in, e.g. num lock
// MASK_1 and _4 hopefully represent alt/meta/windows keys - ymmv?
#define MOD_MASK (XCB_MOD_MASK_SHIFT | XCB_MOD_MASK_LOCK | XCB_MOD_MASK_CONTROL\
        | XCB_MOD_MASK_1 | XCB_MOD_MASK_4)

/* ========================================================================== */

po_window
window_init(size_t width, size_t height)
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

    // Store keyboard symbols
    // TODO: This may need to be strategically run more than once, as
    // technically the keyboard layout could change during runtime.
    // This is valid if I am correctly understanding what I've been reading in
    // the documentation.
    window.keysyms = xcb_key_symbols_alloc(window.connection);

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

void
window_destroy(po_window *window)
{
    xcb_disconnect(window->connection);
    window->connection = NULL;
    // setup is invalidated when the connection is freed
    // Ref: https://xcb.freedesktop.org/PublicApi/#index7h2
    window->setup = NULL;
    // can't find in the docs, but we presume the same is true for screen
    window->screen = NULL;
}

// TODO: Need to refactor this ASAP to do proper event handling
// In particular we need to separate key presses from other event types
po_key
po_key_pressed(po_window *window)
{
    po_key key = PO_KEY_NONE;
    xcb_generic_event_t *event;

    if ((event = xcb_poll_for_event(window->connection))) {
        switch (event->response_type & ~0x80) {
        case XCB_EXPOSE: {
            // TODO: Sensibly re-draw only what's necessary
        } break;

        case XCB_KEY_PRESS: {
            //xcb_key_press_event_t *k = (xcb_key_press_event_t *)event;
            //xcb_keysym_t key_symbol =
            //    xcb_key_press_lookup_keysym(window->keysyms, k, 0);
        } break;

        case XCB_KEY_RELEASE: {
            xcb_key_release_event_t *k = (xcb_key_release_event_t *)event;
            xcb_keysym_t key_symbol =
                xcb_key_release_lookup_keysym(window->keysyms, k, 0);

            if (!(k->state & MOD_MASK) && key_symbol == 'q')
                key = PO_KEY_Q;
        } break;

        default: {
            // Ignore unknown event
        } break;
        }
        // Avoid memory leak
        free(event);
    }

    return key;
}
