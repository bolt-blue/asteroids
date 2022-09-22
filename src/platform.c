// Platform-dependent includes
#ifdef __linux__
#ifndef USE_WAYLAND
#include "x11_platform.c"
#else
#include "wayland_platform.c"
#endif // X11 or Wayland

#elif __unix__
#include "x11_platform.c"

#elif __APPLE__
    // TODO: Apple support (OSX and iOS)

#elif _WIN32
    // TODO: Windows support

#elif __ANDROID__
    // TODO: Android support

#else
#error Platform not supported
#endif // Platform-dependent
