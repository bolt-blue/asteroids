/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef UNIX_PO_WINDOW_H
#define UNIX_PO_WINDOW_H

#ifdef __linux__
#ifndef USE_WAYLAND
#include "x11_po_window.h"
#else
#include "wayland_po_window.h"
#endif

#else // __unix__
#include "x11_po_window.h"
#endif

#endif /* UNIX_PO_WINDOW_H */
