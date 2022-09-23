/*
 * Asteroids (Unix entry point)
 *
 */

#include <dlfcn.h>
#include <stdio.h>

#include "platform.h"

#include "po_utility.h"
#include "po_window.h"

// Dirty unity build
// TODO: Do incremental compile and link instead ?
#include "platform.c"
#include "po_window.c"

#include "asteroids.h"

/* ========================================================================== */

#define SCR_WIDTH 1080
#define SCR_HEIGHT 720

/* ========================================================================== */

typedef struct game_code game_code;
struct game_code {
    GameInit *init;
    GameUpdateAndRender *update_and_render;
};

internal game_code load_game_code(void);
internal void unload_game_code(void);

typedef void lib_handle;
global lib_handle *game_code_lib;

/* ========================================================================== */

/*
 * Memory Layout
 *
 * |=======================|
 * |      draw buffer      |
 * |-----------------------|
 * |                       |
 * |  persistent storage   |
 * |                       |
 * |-----------------------|
 * |                       |
 * |   temporary storage   |
 * |                       |
 * |=======================|
 *
 */
int main(void)
{
    size_t draw_buffer_size = SCR_WIDTH * SCR_HEIGHT * sizeof(po_pixel);
    size_t persistent_storage_size = MB(4);
    size_t temporary_storage_size = MB(4);
    size_t total_size = draw_buffer_size + persistent_storage_size + temporary_storage_size;

    po_memory memory = po_map_mem(total_size);

    po_window window = {0};
    window = po_window_init(SCR_WIDTH, SCR_HEIGHT);

    // TODO This check should be handled internally
    // We need a good way to retrieve any error status here
    if (!window.connection) {
        LOG_ERROR("Failed to initialise our window. Exiting.");
        return 1;
    }

    game_input controller_input = {0};

    offscreen_draw_buffer draw_buffer = {
        .width = SCR_WIDTH, .height = SCR_HEIGHT,
        .data = memory.base
    };

    // The window directly references the draw buffer
    window.buffer = draw_buffer.data;

    game_code game = load_game_code();

    void *game_base = memory.base + draw_buffer_size;
    game.init(game_base, persistent_storage_size, temporary_storage_size, &draw_buffer);

    uint8_t done = 0;
    struct timespec begin, end;
    struct timespec delta;
    struct timespec pause = {0};

    while (!done)
    {
        clock_gettime(CLOCK_MONOTONIC, &begin);

        // Process input
        po_get_input_state(&window, &controller_input);
        if (controller_input.quit) done = 1;

        // TODO: Make use of return value here
        game.update_and_render(game_base, &controller_input, &draw_buffer);

        clock_gettime(CLOCK_MONOTONIC, &end);

        po_timespec_diff(&end, &begin, &delta);
        pause.tv_nsec = PULSE - delta.tv_nsec;

        // TODO: During testing, there is a regular period where this gets
        // triggered. Determine cause and go from there
        if (delta.tv_nsec > PULSE) {
            LOG_WARN("Target FPS exceeded! [%.2fms > %.2fms]",
                    NSTOMS(delta.tv_nsec), NSTOMS(PULSE));
        }

#if 1
        LOG_DEBUG("Trgt: %.2fms | Dt: %5.2fms | Sl: %5.2fms",
                NSTOMS(PULSE),
                NSTOMS(delta.tv_nsec),
                NSTOMS(pause.tv_nsec));
#endif

        nanosleep(&pause, NULL);

        // Blit
        po_render_to_screen(&window);
    }

    // Clean up
    // NOTE: Essentially we're only doing this to keep valgrind (et al) happy
    // TODO: Avoid doing this in production
    unload_game_code();
    po_window_destroy(&window);
    po_unmap_mem(&memory);

    return 0;
}

/* ========================================================================== */

game_code
load_game_code(void)
{
    game_code_lib = dlopen("asteroids.so", RTLD_NOW);
    if (!game_code_lib) {
        LOG_ERROR("Failed to load core game code: %s", dlerror());
    }

    game_code game_lib = {0};

    game_lib.init = (GameInit *)dlsym(game_code_lib, "game_init");
    game_lib.update_and_render = (GameUpdateAndRender *)dlsym(game_code_lib, "game_update_and_render");

    if (!game_lib.init)
        game_lib.init = game_init_stub;
    if (!game_lib.update_and_render)
        game_lib.update_and_render = game_update_and_render_stub;

    return game_lib;
}

void
unload_game_code()
{
    dlclose(game_code_lib);
}
