/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef PO_STACK_H
#define PO_STACK_H

#include "po_arena.h"
#include "po_types.h"

/* ========================================================================== */

typedef struct po_stack po_stack;
struct po_stack {
    size_t max;
    size_t top;
    size_t member_size;
    int8_t *data;
};

/* ========================================================================== */

po_stack po_stack_create(size_t capacity, size_t member_size, po_arena *arena);
int po_stack_push(po_stack *stack, void *value);
void *po_stack_pop(po_stack *stack);

#endif /* PO_STACK_H */
