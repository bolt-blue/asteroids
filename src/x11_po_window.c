#include <stdlib.h>  // free (seems a bit exessive, but oh well)

#include "po_window.h"

// TODO: Error handling

// Mask for the common key modifiers, ignoring those we're generally not
// interested in, e.g. num lock
// MASK_1 and _4 hopefully represent alt/meta/windows keys - ymmv?
#define MOD_MASK (XCB_MOD_MASK_SHIFT | XCB_MOD_MASK_LOCK | XCB_MOD_MASK_CONTROL\
        | XCB_MOD_MASK_1 | XCB_MOD_MASK_4)

#define ESC 0xff1b // keycode 9

/* ========================================================================== */

/*
 * Create and initialise a window
 *
 * In this context and window is mainly a means to having a plain old drawing
 * surface.
 *
 * At present the surface is automatically created and matches the requested
 * dimensions for the window. This may or may not remain as the default case.
 *
 */
po_window
po_window_init(uint16_t width, uint16_t height, po_arena *arena)
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
    window.id = xcb_generate_id(window.connection);
    xcb_create_window(window.connection,
            XCB_COPY_FROM_PARENT,                   /* depth (same as root) */
            window.id,                              /* window id            */
            window.screen->root,                    /* parent window        */
            0, 0,                                   /* x, y (ignored by WM) */
            window.width, window.height,            /* width, height        */
            10,                                     /* border_width         */
            XCB_WINDOW_CLASS_INPUT_OUTPUT,          /* class                */
            window.screen->root_visual,             /* visual               */
            masks, mask_values);                    /* masks                */

    // Create surface (pixmap)
    po_surface surface = (po_surface){.id = xcb_generate_id(window.connection),
            .width = window.width, .height = window.height};

    surface.data = po_arena_push(surface.width * surface.height *
            sizeof(*surface.data), arena);

    // NOTE: Currently we're blitting directly to the window
    // See TODO in draw_surface()
    xcb_create_pixmap(window.connection, window.screen->root_depth,
            surface.id, window.id,
            surface.width, surface.height);

    // Create graphics context
    surface.gc = xcb_generate_id(window.connection);
    xcb_create_gc(window.connection, surface.gc, surface.id, 0, NULL);

    // Store surface details for later
    window.surface = po_arena_push(sizeof(po_surface), arena);
    *window.surface = surface;

    // Draw the window
    xcb_map_window(window.connection, window.id);
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

/*
 * Record the state of our controller input
 * We only track the essentials
 *
 * TODO: Utilise the return value for error handling
 */
int
po_get_input_state(po_window *window, game_input *input)
{
    xcb_generic_event_t *event;

    while ((event = xcb_poll_for_event(window->connection))) {
        switch (event->response_type & ~0x80) {
        case XCB_EXPOSE: {
            // TODO: Sensibly re-draw only what's necessary
        } break;

        case XCB_KEY_PRESS: {
            xcb_key_press_event_t *k = (xcb_key_press_event_t *)event;
            xcb_keysym_t key_symbol =
                xcb_key_press_lookup_keysym(window->keysyms, k, 0);
            if (!(k->state & MOD_MASK)) {
                switch (key_symbol)
                {
                case 'w': input->thrust.is_down = 1; break;
                case 'a': input->left.is_down   = 1; break;
                case 'd': input->right.is_down  = 1; break;
                case 'h': input->hyper.is_down  = 1; break;
                case ' ': input->fire.is_down   = 1; break;
                }
            }
        } break;

        case XCB_KEY_RELEASE: {
            xcb_key_release_event_t *k = (xcb_key_release_event_t *)event;
            xcb_keysym_t key_symbol =
                xcb_key_release_lookup_keysym(window->keysyms, k, 0);

            if (!(k->state & MOD_MASK)) {
                switch (key_symbol)
                {
                case 'w': input->thrust.is_down = 0; break;
                case 'a': input->left.is_down   = 0; break;
                case 'd': input->right.is_down  = 0; break;
                case 'h': input->hyper.is_down  = 0; break;
                case ' ': input->fire.is_down   = 0; break;
                case ESC: input->quit           = 1; break;
                }
            }
            // NOTE: Alternatively the ESC key could be detected directly via
            // its keycode rather than symbol, by checking k->detail instead
        } break;

        default: {
            // Ignore unknown event
        } break;
        }
        // Avoid memory leak
        // TODO: Can we do any better than this?
        free(event);
    }

    return 0;
}

void
po_render_surface(po_window *window)
{
    po_surface *surface = window->surface;
    // Write from our surface buffer directly to the window
    // TODO: First write to our surface pixmap - double buffering
    xcb_put_image(window->connection, XCB_IMAGE_FORMAT_Z_PIXMAP,
            window->id, surface->gc, surface->width, surface->height,
            0, 0, 0, window->screen->root_depth,
            surface->width * surface->height * sizeof(*surface->data),
            (uint8_t *)(surface->data));

#if 0
    // Copy from the pixmap to the window
    xcb_copy_area(window->connection,
            surface->id, window->id, surface->gc,
            0, 0, 0, 0, surface->width, surface->height);
#endif
    xcb_flush(window->connection);
}
