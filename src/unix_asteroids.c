/*
 * Asteroids (Unix entry point)
 *
 */

#include <stdio.h>

// Dirty unity build
// TODO: Do incremental compile and link instead ?
#include "asteroids.c"
#include "po_utility.h"
#include "po_window.c"

/* ========================================================================== */

int main(void)
{
    po_context context = {0};

    context.game_memory.persistent_memory = po_arena_create(MB(4));
    context.game_memory.temporary_memory = po_arena_create(MB(4));

    context.window = po_arena_push(sizeof(po_window), &context.game_memory.persistent_memory);
    *context.window = po_window_init(1080, 720, &context.game_memory.persistent_memory);

    // TODO This check should be handled internally
    // We need a good way to retrieve any error status here
    if (!context.window->connection) {
        LOG_ERROR("Failed to initialise our window. Exiting.");
        return 1;
    }

    // TODO: Do event gathering here, then pass them to the game code directly
    // The game code should not have to call into the platform layer
    game_input controller_input = {0};

    uint8_t done = 0;
    struct timespec begin, end;
    struct timespec delta;
    struct timespec pause = {0};

    while (!done)
    {
        clock_gettime(CLOCK_MONOTONIC, &begin);

        // Process input
        int quit = po_get_input_state(context.window, &controller_input);
        if (quit) done = 1;

        // TODO: Make use of return value here
        // TODO: Pass only the necessities
        // - input
        // - draw buffer
        // - audio buffer
        game_update_and_render(&context, &controller_input);

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
        po_render_surface(context.window);
    }

    // Clean up
    po_window_destroy(context.window);
    po_arena_destroy(&context.game_memory.persistent_memory);

    return 0;
}
