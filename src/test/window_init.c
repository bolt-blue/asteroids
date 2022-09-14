/*
 * Test to see that we can get a window drawn to screen
 */

// Dirty unity build
#include "../po_window.c"
#include <unistd.h> // TMP sleep

int main(void)
{
    po_window win = window_init(1080, 720);

    // TODO: Exit on 'q' keypress
    sleep(2);

    // Clean up
    window_destroy(&win);

    return 0;
}
