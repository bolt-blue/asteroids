#include "po_arena.h"
#include "po_utility.h"

#ifndef NDEBUG
#include <stdio.h>  // ASSERT
#endif

po_arena
po_arena_init(size_t size, void *base)
{
    return (po_arena){.capacity = size, .top = 0, .base = base};
}

void *
po_arena_push(size_t size, po_arena *arena)
{
    ASSERT(arena);
    ASSERT(arena->base);
    // TODO: Guard against overflow
    // NOTE: The game is designed to never exceed it's memory limits
    ASSERT(arena->top + size <= arena->capacity);

    if (!size) return NULL;

    void *result = arena->base + arena->top;
    arena->top += size;
    return result;
}

inline void
po_arena_clear(po_arena *arena)
{
    arena->top = 0;
}
