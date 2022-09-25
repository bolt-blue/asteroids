/*
 * Asteroids (Unix entry point)
 *
 */

#ifdef INTERNAL_BUILD
#include <sys/stat.h>
#include <unistd.h>
#endif

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

// TODO: Learn how this magic works
// This gives us a type that plays nicely with e.g. dlclose(), but will nicely
// allow the compiler to warn us if we try to use it where we shouldn't
typedef void (*lib_handle)(const char *, int);

typedef struct game_code game_code;
struct game_code {
    GameInit *init;
    GameUpdateAndRender *update_and_render;

    lib_handle *game_code_handle;
#ifdef INTERNAL_BUILD
    struct timespec game_last_mod_time;
#endif
};

internal game_code load_game_code(const char *lib_path, const char *debug_lib_path);
internal void unload_game_code(game_code *game);
internal void cat_str(size_t read_len_a, const char *src_a,
        size_t read_len_b, const char *src_b,
        size_t write_len, char *dst);
#ifdef INTERNAL_BUILD
internal inline struct timespec file_mod_time(const char *file_path);
internal inline bool32 null_time(struct timespec ts);
internal inline bool32 times_differ(struct timespec a, struct timespec b);
internal int copy_file(const char *src_path, const char *dst_path);
#endif

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
int main(int argc, char **argv)
{
    char *bin_path = argv[0];
    size_t base_path_len = 0;
    for (size_t pos = 0; bin_path[pos]; pos++)
    {
        if (bin_path[pos] == '/')
            base_path_len = pos + 1;
    }

    const char lib_name[] = "asteroids.so";
    size_t lib_name_len = sizeof(lib_name) - 1;
    size_t full_path_len = base_path_len + lib_name_len;
    char lib_path[full_path_len + 1];
    cat_str(base_path_len, bin_path,
            lib_name_len, lib_name,
            full_path_len, lib_path);

#ifndef INTERNAL_BUILD
    const char *debug_lib_path = NULL;
#else
    const char debug_lib_name[] = "asteroids.dbg.so";
    size_t debug_lib_name_len = sizeof(debug_lib_name) - 1;
    size_t full_debug_path_len = base_path_len + debug_lib_name_len;
    char debug_lib_path[full_debug_path_len + 1];
    cat_str(base_path_len, bin_path,
            debug_lib_name_len, debug_lib_name,
            full_debug_path_len, debug_lib_path);
#endif

    size_t draw_buffer_size = SCR_WIDTH * SCR_HEIGHT * sizeof(po_pixel);
    size_t persistent_storage_size = MB(4);
    size_t temporary_storage_size = MB(4);
    size_t total_size = draw_buffer_size + persistent_storage_size + temporary_storage_size;

    po_memory memory = po_map_mem(total_size);

    po_window window = {};
    window = po_window_init(SCR_WIDTH, SCR_HEIGHT);

    // TODO This check should be handled internally
    // We need a good way to retrieve any error status here
    if (!window.connection) {
        LOG_ERROR("Failed to initialise our window. Exiting.");
        return 1;
    }

    game_input controller_input = {};

    offscreen_draw_buffer draw_buffer = {
        .width = SCR_WIDTH, .height = SCR_HEIGHT,
        .data = memory.base
    };

    // The window directly references the draw buffer
    window.buffer = draw_buffer.data;

    game_code game = load_game_code(lib_path, debug_lib_path);

    void *game_base = (int8_t *)memory.base + draw_buffer_size;
    game.init(game_base, persistent_storage_size, temporary_storage_size, &draw_buffer);

    uint8_t done = 0;
    struct timespec begin, end;
    struct timespec delta;
    struct timespec pause = {};

    while (!done)
    {
        clock_gettime(CLOCK_MONOTONIC, &begin);

#ifdef INTERNAL_BUILD
        struct timespec game_current_mod_time = file_mod_time(lib_path);
        if (!null_time(game_current_mod_time)
                && times_differ(game_current_mod_time, game.game_last_mod_time))
        {
            LOG_DEBUG("=== New game core detected");
            LOG_DEBUG("    Current: %ld.%ld\n"
                    "            Last:    %ld.%ld",
                    game_current_mod_time.tv_sec, game_current_mod_time.tv_nsec,
                    game.game_last_mod_time.tv_sec, game.game_last_mod_time.tv_nsec);

            unload_game_code(&game);
            game = load_game_code(lib_path, debug_lib_path);
        }
#endif

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

#if 0
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
    unload_game_code(&game);
    po_window_destroy(&window);
    po_unmap_mem(&memory);

    return 0;
}

/* ========================================================================== */

internal game_code
load_game_code(const char *lib_path, const char *debug_lib_path)
{
    game_code game_lib = {};

    lib_handle *game_code_handle = 0;

#ifndef INTERNAL_BUILD
    game_code_handle = dlopen(lib_path, RTLD_NOW | RTLD_LOCAL);

#else
    // We copy the core game code and run from there, which plays things more
    // safely when we re-compile and hot load during runtime
    // TODO: Do we really need to do the copy for unix?
    if (copy_file(lib_path, debug_lib_path) != 0)
    {
        LOG_DEBUG("=== Failed to copy game core");
    }
    else
    {
        game_code_handle = dlopen(debug_lib_path, RTLD_NOW | RTLD_LOCAL);
        if (game_code_handle)
        {
            LOG_DEBUG("=== Loaded game core\n");
            game_lib.game_last_mod_time = file_mod_time(lib_path);
        }
    }
#endif

    if (!game_code_handle)
        LOG_ERROR("Failed to load game core: %s", dlerror());

    game_lib.game_code_handle = game_code_handle;

    game_lib.init = (GameInit *)
        dlsym(game_code_handle, "game_init");
    game_lib.update_and_render = (GameUpdateAndRender *)
        dlsym(game_code_handle, "game_update_and_render");

    if (!game_lib.init)
        game_lib.init = game_init_stub;
    if (!game_lib.update_and_render)
        game_lib.update_and_render = game_update_and_render_stub;

    return game_lib;
}

internal void
unload_game_code(game_code *game)
{
    if (game->game_code_handle) {
        dlclose(game->game_code_handle);
        game->game_code_handle = 0;
    }
    game->init = 0;
    game->update_and_render = 0;
}

/*
 * Copy read_len_a bytes from src_a followed by read_len_b bytes from src_b
 * into dst
 *
 * It is assumed that dst has sufficient space to store write_len characters
 * plus one (for the nul terminator)
 *
 * The nul terminator will be added after both sources have been concatenated,
 * or when write_len has been reached, whichever comes first
 */
internal void
cat_str(size_t read_len_a, const char *src_a,
        size_t read_len_b, const char *src_b,
        size_t write_len, char *dst)
{
    while (read_len_a--)
    {
        if (!write_len) break;
        *dst++ = *src_a++;
        write_len--;
    }
    while (read_len_b--)
    {
        if (!write_len) break;
        *dst++ = *src_b++;
        write_len--;
    }
    *dst = '\0';
}

#ifdef INTERNAL_BUILD
internal inline struct timespec
file_mod_time(const char *file_path)
{
    struct stat file_stat;
    if (stat(file_path, &file_stat) != 0) {
        return (struct timespec){};
    }
    return file_stat.st_mtim;
}

internal inline bool32
null_time(struct timespec ts)
{
    return (ts.tv_sec == 0 && ts.tv_nsec == 0);
}

/*
 * Returns
 * 1: When times differ
 * 0: Otherwise
 */
internal inline bool32
times_differ(struct timespec a, struct timespec b)
{
    return (a.tv_sec != b.tv_sec) || (a.tv_nsec != b.tv_nsec);
}

#include <errno.h>
internal int
copy_file(const char *src_path, const char *dst_path)
{
    if (src_path == dst_path)
        return -1;

    FILE *src = fopen(src_path, "r");
    if (!src)
        return -2;

    FILE *dst = fopen(dst_path, "w");
    if (!dst) {
        fclose(src);
        return -2;
    }

#define COPY_BUF_SZ 4096
    char buffer[COPY_BUF_SZ];
    uint32_t bytes_read;

    LOG_DEBUG("=== Copying game core");

    while ((bytes_read = fread(buffer, 1, COPY_BUF_SZ, src)))
    {
        fwrite(buffer, bytes_read, 1, dst);
    }

    if (ferror(src))
        LOG_DEBUG("    Read error during copy: %d", ferror(src));

    fclose(src);
    fclose(dst);

    return 0;
}
#endif
