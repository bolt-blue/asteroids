#include "po_utility.h"
#include "po_window.h"

#include <stdio.h>
#include <sys/mman.h>

po_arena
po_arena_create(size_t size)
{
    po_arena arena = {.capacity = size};
    arena.data = mmap(0, size,
            PROT_READ | PROT_WRITE,
            MAP_ANONYMOUS | MAP_PRIVATE,
            -1, 0);
    return arena;
}

void
po_arena_destroy(po_arena *arena)
{
    if (!arena) return;
    if (!arena->data) return;
    munmap(arena->data, arena->capacity);
    *arena = (po_arena){0};
}

void *
po_arena_push(size_t size, po_arena *arena)
{
    ASSERT(arena);
    ASSERT(arena->data);
    // TODO: Guard against overflow
    // NOTE: The game is designed to never exceed it's memory limits
    ASSERT(arena->top + size <= arena->capacity);

    if (!size) return NULL;

    void *result = arena->data + arena->top;
    arena->top += size;
    return result;
}
