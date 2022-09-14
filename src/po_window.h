/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef PO_WINDOW_H
#define PO_WINDOW_H

/*
 * TODO: Keep this comment up to date
 * For linux, we will build for both X11 and Wayland (eventually)
 * We start with and default to X11
 * If you want to compile using Wayland, set USE_WAYLAND during your chosen
 * build method
 */

// This struct must be defined separately by each platform layer
// TODO: Should this be an opaque type?
typedef struct po_window po_window;

#ifdef __linux__
#include "linux_po_window.h"
#elif __APPLE__
    // TODO: Apple support (OSX and iOS)
#elif _WIN32
    // TODO: Windows support
#elif __ANDROID__
    // TODO: Android support
#else
#error Platform not supported
#endif // Platform

#endif /* PO_WINDOW_H */
