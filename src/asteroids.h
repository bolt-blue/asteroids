/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include "platform.h"

#include "po_stack.h"
#include "po_types.h"
#include "po_utility.h"
#include "po_vector.h"

/* ========================================================================== */

// WARNING: Don't be silly and pass an expression with side-effects
// TODO: This should work with negative numbers as well
#define ABS(v) ((v) + ((v) < 0) * -(v) * 2)
#define ROUND(v) (int)((float)(v) + 0.5)

#define PI 3.14159265358979323846
#define TWOPI (PI * 2)

/* ========================================================================== */

typedef struct po_line po_line;
struct po_line {
    point2 va;
    point2 vb;
    uint32_t thickness;
};

#define LINE(x0, y0, x1, y1) (po_line){.va.x = (x0), .va.y = (y0), .vb.x = (x1), .vb.y = (y1)}
#define CREATE_LINE_STACK(cap, arena_ptr) po_stack_create((cap), sizeof(po_line), (arena_ptr))
inline int line_stack_push(po_stack *stack, po_line line);
inline po_line *line_stack_pop(po_stack *stack);

typedef struct ship ship;
struct ship {
    // TODO: Move to using quaternions at some point
    vec2 position;
    vec2 acceleration;
    vec2 velocity;
    float heading;  // Radians
    float previous_heading;
    uint32_t line_count;
    po_line *lines;
};


typedef struct game_state game_state;
struct game_state {
    ship the_ship;

    po_arena persistent_memory;
    po_arena temporary_memory;
};

#endif /* ASTEROIDS_H */
