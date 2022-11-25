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
    // TODO: Implement
    //uint32_t thickness;
};

#define LINE(x0, y0, x1, y1) (po_line){.va.x = (x0), .va.y = (y0), .vb.x = (x1), .vb.y = (y1)}
#define CREATE_LINE_STACK(cap, arena_ptr) po_stack_create((cap), sizeof(po_line), (arena_ptr))
inline int line_stack_push(po_stack *stack, po_line line);
inline po_line *line_stack_pop(po_stack *stack);

typedef uint32_t object_id;

typedef struct ship ship;
struct ship {
    object_id id;

    vec2 acceleration;

    float heading;  // Radians
    float previous_heading;
};

typedef struct asteroid asteroid;
struct asteroid {
    object_id id;

    //float heading;
    // TODO: Use the mass to reasonably handle conservation of energy and
    // momentum upon collisions between asteroids
    //float mass;
};

typedef struct drawable drawable;
struct drawable {
    uint32_t line_count;
    po_line *lines;
};

/*
 * SoA for properties shared by all of the key game objects;
 * the player ship, the enemy saucers and the asteroids
 *
 * Each array holds count items
 *
 * An object_id is an index into each array
 */
typedef struct objects_in_space objects_in_space;
struct objects_in_space {
    uint32_t count;
    uint32_t capacity;

    // TODO: Move to using quaternions at some point

    vec2 *positions;
    vec2 *velocities;

    // Currently only the player ship requires an orientation
    // TODO: Make use of orientation for asteroids as well
    //float *orientations;    // in radians

    // The actual defined shapes - stable across frames
    // TODO: Switch to using VAO's
    drawable *shapes;

    // Per-frame shape segments, potentially split across boundaries
    drawable *split_shapes;
};

typedef struct game_state game_state;
struct game_state {
    ship *the_ship;

    size_t asteroid_count;
    asteroid *asteroids;

    objects_in_space *objects;

    po_arena persistent_memory;
    po_arena temporary_memory;
};

#endif /* ASTEROIDS_H */
