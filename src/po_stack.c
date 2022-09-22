#include <stddef.h>  // size_t
#ifndef NDEBUG
#include <stdio.h>   // For our ASSERT macro
#endif

#include "po_stack.h"

#include "po_arena.h"
#include "po_utility.h"

internal inline void mem_copy(void *dst, void *src, size_t amt);

po_stack
po_stack_create(size_t capacity, size_t member_size, po_arena *arena)
{
    po_stack result = (po_stack){.max = capacity, .member_size = member_size,
        result.data = po_arena_push(capacity * member_size, arena)};
    return result;
}

int
po_stack_push(po_stack *stack, void *value)
{
    // TODO: Maybe we change this in future. For now though, we should only
    // create and use stacks of known sufficient and manageable sizes
    ASSERT(stack->top < stack->max);
    //if (stack->top == stack->max) return 1;
    size_t pos = stack->top * stack->member_size;
    mem_copy(&stack->data[pos], value, stack->member_size);
    stack->top++;
    return 0;
}

void *
po_stack_pop(po_stack *stack)
{
    if (stack->top == 0) return NULL;
    stack->top--;
    size_t pos = stack->top * stack->member_size;
    return &stack->data[pos];
}

/*
 * Copy amt bytes from src to dst
 *
 * WARNING:
 * Don't call this with overlapping memory
 */
void
mem_copy(void *dst, void *src, size_t amt)
{
    int8_t *dbyte = dst, *sbyte = src;
    while (amt--) *dbyte++ = *sbyte++;
}
