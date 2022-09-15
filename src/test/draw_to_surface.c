/*
 * Test to see that we can create a surface, draw to it and
 * display it to our window
 */

#include <stdint.h>
#include <stdio.h>
#include <time.h>

// Dirty unity build
// TODO: Do incremental compile and link instead ?
#include "../po_utility.h"
#include "../po_window.c"

/* ========================================================================== */

#define PULSE SIXTY_FPS

/* ========================================================================== */

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

int main(void)
{
    po_window window = po_window_init(1080, 720);

    if (!window.connection) {
        LOG_ERROR("Failed to initialise our window. Exiting.");
        return 1;
    }

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

        update_surface(&window.surface);
        po_render_surface(&window);

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
    po_window_destroy(&window);

    return 0;
}
