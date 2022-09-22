#ifdef __unix__
#include "unix_po_arena.c"
#endif

#ifdef __linux__
#ifndef USE_WAYLAND
#include "x11_po_window.c"
#else
#include "wayland_po_window.c"
#endif // X11 or Wayland

#elif __APPLE__
    // TODO: Apple support (OSX and iOS)

#elif _WIN32
    // TODO: Windows support

#elif __ANDROID__
    // TODO: Android support

#else
#error Platform not supported
#endif // Platform

#include "po_stack.c"
