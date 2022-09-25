/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef PO_ARENA_H
#define PO_ARENA_H

#include "po_types.h"

/* ========================================================================== */

typedef struct po_arena po_arena;
struct po_arena {
    uint32_t capacity;
    uint32_t top;
    uint8_t *base;
};

/* ========================================================================== */

po_arena po_arena_init(size_t size, void *base);
void *po_arena_push(size_t size, po_arena *arena);
void po_arena_clear(po_arena *arena);

/* ========================================================================== */

#define PUSH_STRUCT(type, arena) po_arena_push(sizeof(type), arena)
#define PUSH_ARRAY(type, count, arena) po_arena_push((count) * sizeof(type), arena)

#endif /* PO_ARENA_H */
