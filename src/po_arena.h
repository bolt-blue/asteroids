/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef PO_ARENA_H
#define PO_ARENA_H

#include <stddef.h>
#include <stdint.h>

/* ========================================================================== */

typedef struct po_arena po_arena;
struct po_arena {
    uint32_t capacity;
    uint32_t top;
    uint8_t *data;
};

/* ========================================================================== */

po_arena po_arena_create(size_t size);
void po_arena_destroy(po_arena *arena);
void *po_arena_push(size_t size, po_arena *arena);
void po_arena_clear(po_arena *arena);

#endif /* PO_ARENA_H */
