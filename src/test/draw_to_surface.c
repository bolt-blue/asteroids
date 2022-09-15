/*
 * Test to see that we can create a surface, draw to it and
 * display it to our window
 */

#include <stdint.h>
#include <stdio.h>
#include <time.h>

// Dirty unity build
#include "../po_utility.h"
#include "../po_window.c"

/* ========================================================================== */

#define PULSE SIXTY_FPS

/* ========================================================================== */

// TODO: Separate out surface stuff

/*
 * We use an RGBA bitmap format
 */
typedef struct po_pixel po_pixel;
struct po_pixel {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
};

typedef struct po_surface po_surface;
struct po_surface {
    xcb_pixmap_t id;
    xcb_gcontext_t gc;
    size_t width;
    size_t height;
    po_pixel *data;
};

void update_surface(po_surface *surface)
{
    // Weird shifting gradient thing
    static uint8_t start = 0;
    static uint8_t start_step = 1;
    uint8_t step = surface->height / 255.0f + 0.5;

    start += start_step;
    if (start == 255 || start == 0) start_step = -start_step;

    po_pixel colour_left = {255, start, 0};
    po_pixel colour_right = {start, 0, 255};

    for (int i = 0; i < surface->height; i++)
    {
        size_t row_pos = i * surface->width;

        if (!(i % step)) {
            colour_left.b--;
            colour_left.r++;
            colour_right.g++;
            colour_right.r--;
        }

        po_pixel colour;
        for (int j = 0; j < surface->width; j++)
        {
            if (j < surface->width / 2) {
                colour = colour_left;
            } else {
                colour = colour_right;
            }
            surface->data[row_pos + j] = colour;
        }
    }
}

void draw_surface(po_surface *surface, po_window *window)
{
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

int main(void)
{
    po_window window = window_init(1080, 720);

    // Create surface (pixmap)
    po_surface surface = {.id = xcb_generate_id(window.connection),
                          .width = window.width, .height = window.height};
    surface.data = malloc(surface.width * surface.height * sizeof(*surface.data));
    if (!surface.data) {
        LOG_ERROR("Failed to allocate memory for our drawing surface. Exiting.");
        return 1;
    }

    // NOTE: Currently we're blitting directly to the window
    // See TODO in draw_surface()
    xcb_create_pixmap(window.connection, window.screen->root_depth,
            surface.id, window.id, surface.width, surface.height);

    // Create graphics context
    surface.gc = xcb_generate_id(window.connection);
    xcb_create_gc(window.connection, surface.gc, surface.id, 0, NULL);

    uint8_t done = 0;
    struct timespec begin, end;
    struct timespec delta;
    struct timespec pause = {0};

    while (!done)
    {
        clock_gettime(CLOCK_MONOTONIC, &begin);

        if (po_key_pressed(&window) == PO_KEY_Q) {
            break;
        }

        update_surface(&surface);
        draw_surface(&surface, &window);

        clock_gettime(CLOCK_MONOTONIC, &end);

        po_timespec_diff(&end, &begin, &delta);
        pause.tv_nsec = PULSE - delta.tv_nsec;

        // TODO: During testing, there is a regular period where this gets
        // triggered. Determine cause and go from there
        if (delta.tv_nsec > PULSE) {
            LOG_WARN("Target FPS exceeded! [%.2fms > %.2fms]",
                    NSTOMS(delta.tv_nsec), NSTOMS(PULSE));
        }

        LOG_DEBUG("Trgt: %.2fms | Dt: %5.2fms | Sl: %5.2fms",
                NSTOMS(PULSE),
                NSTOMS(delta.tv_nsec),
                NSTOMS(pause.tv_nsec));

        nanosleep(&pause, NULL);
    }

    // Clean up
    free(surface.data);
    xcb_key_symbols_free(window.keysyms);
    window_destroy(&window);

    return 0;
}
