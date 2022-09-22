/*
 * Asteroids (Unix entry point)
 *
 */

#include <stdio.h>

#include "platform.h"

#include "po_utility.h"
#include "po_window.h"

// Dirty unity build
// TODO: Do incremental compile and link instead ?
#include "platform.c"
#include "unix_po_arena.c"
#include "po_window.c"

#include "asteroids.h"
// DEBUG @tmp
#include "asteroids.c"

/* ========================================================================== */

#define SCR_WIDTH 1080
#define SCR_HEIGHT 720

int main(void)
{
    game_memory memory;
    memory.persistent_memory = po_arena_create(MB(4));
    memory.temporary_memory = po_arena_create(MB(4));

    po_window *window = po_arena_push(sizeof(po_window), &memory.persistent_memory);
    *window = po_window_init(SCR_WIDTH, SCR_HEIGHT, &memory.persistent_memory);

    // TODO This check should be handled internally
    // We need a good way to retrieve any error status here
    if (!window->connection) {
        LOG_ERROR("Failed to initialise our window. Exiting.");
        return 1;
    }

    game_input controller_input = {0};

    offscreen_draw_buffer draw_buffer = {
        .width = SCR_WIDTH, .height = SCR_HEIGHT,
        .data = po_arena_push(SCR_WIDTH * SCR_HEIGHT * sizeof(po_pixel),
                &memory.persistent_memory)
    };

    // The window directly references the draw buffer
    window->buffer = draw_buffer.data;

    uint8_t done = 0;
    struct timespec begin, end;
    struct timespec delta;
    struct timespec pause = {0};

    while (!done)
    {
        clock_gettime(CLOCK_MONOTONIC, &begin);

        // Process input
        po_get_input_state(window, &controller_input);
        if (controller_input.quit) done = 1;

        // TODO: Make use of return value here
        game_update_and_render(&memory, &controller_input, &draw_buffer);

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

        // Blit
        po_render_to_screen(window);
    }

    // Clean up
    po_window_destroy(window);
    po_arena_destroy(&memory.persistent_memory);

    return 0;
}
