/*
 * Test to see that we can get a window drawn to screen
 */

#include <stdint.h>
#include <time.h>

// Dirty unity build
#include "../po_utility.h"
#include "../po_window.c"

/* ========================================================================== */

#define PULSE THIRTY_FPS

/* ========================================================================== */

int main(void)
{
    po_window window = window_init(1080, 720);

    uint8_t done = 0;
    struct timespec pause = {.tv_nsec = PULSE};

    while (!done)
    {
        if (po_key_pressed(&window) == PO_KEY_Q) {
            done = 1;
        }

        nanosleep(&pause, NULL);
    }

    // Clean up
    xcb_key_symbols_free(window.keysyms);
    window_destroy(&window);

    return 0;
}
