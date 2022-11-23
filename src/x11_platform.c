#include "platform.h"

#include <stdlib.h>     // free
#include <sys/mman.h>   // mmap, munmap

/* ========================================================================== */

// Mask for the common key modifiers, ignoring those we're generally not
// interested in, e.g. num lock
// MASK_1 and _4 hopefully represent alt/meta/windows keys - ymmv?
#define MOD_MASK (XCB_MOD_MASK_SHIFT | XCB_MOD_MASK_LOCK | XCB_MOD_MASK_CONTROL\
        | XCB_MOD_MASK_1 | XCB_MOD_MASK_4)

#define ESC 0xff1b // keycode 9

/* ========================================================================== */

po_memory po_map_mem(size_t size, void *address)
{
    // TODO: Map to a consistent base address (at least during development)
    // It will allow for some useful tooling later
    po_memory mapped = {.size = size,
        .base = mmap(address, size,
            PROT_READ | PROT_WRITE,
            MAP_ANONYMOUS | MAP_PRIVATE,
            -1, 0)
    };
    return mapped;
}

void po_unmap_mem(po_memory *memory)
{
    if (!memory) return;
    if (!memory->base) return;
    munmap(memory->base, memory->size);
    *memory = (po_memory){0};
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
po_render_to_screen(po_window *window)
{
    // Write from our buffer directly
    // TODO: First write to our pixmap - double buffering ?
    xcb_put_image(window->connection, XCB_IMAGE_FORMAT_Z_PIXMAP,
            window->win_id, window->gc_id, window->width, window->height,
            0, 0, 0, window->screen->root_depth,
            window->width * window->height * sizeof(*window->buffer),
            (uint8_t *)(window->buffer));

#if 0
    // Copy from the pixmap to the window
    xcb_copy_area(window->connection,
            window->pm_id, window->win_id, window->gc_id,
            0, 0, 0, 0, window->width, window->height);
#endif
    xcb_flush(window->connection);
}
