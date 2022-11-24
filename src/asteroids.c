/*
 * Asteroids
 *
 * A humble clone of Atari's arcade classic.
 *
 */

// TODO: Switch to using shortened typenames, e.g. u8
#if !defined(NDEBUG) || !defined(NTRACE)
#include <stdio.h>  // ASSERT, *_LOG
#endif

#include "asteroids.h"

// Unity build
// TODO: Do incremental compile and link instead ?
#include "po_arena.c"
#include "po_stack.c"
#include "po_vector.c"

/* ========================================================================== */

// Acceleration is a constant factor
#define THRUST_QTY 10

/*
 * In the original game, the maximum number of large asteroids that can spawn
 * on-screen is 10. If all of those were split into medium and then small,
 * without any being fully vapourised, that gives us a limit of 40 (both large
 * and medium split into two upon being shot)
 * NOTE: Given that this is a trivial quantity, we can simply allocate all the
 * necessary memory, even though it will almost certainly never be fully used
 */
#define MAX_ASTEROIDS 40

/*
 * Back of the napkin calculations for max possible objects on screen:
 *   40 asteroids
 *   1 ship
 *   2 saucers
 *   9 shots fired (3 from each ship/saucer)
 * = 52
 *
 * TODO: This will need re-evaluating as we go along, at least with regard to
 * particle effects
 */
#define MAX_OBJECTS 64

/* ========================================================================== */

internal void rotate_lines(size_t n, po_line lines[n], float rad);

internal po_line *line_divide(po_line line, int xmin, int xmax, int ymin, int ymax,
        po_arena *arena, size_t *out_count);

#if 0
internal void po_memset(void *mem, int c, size_t n)
{
    uint8_t *m = mem;
    while (n--) *m++ = c;
}
#endif

/* ========================================================================== */

internal void
clear_draw_buffer(offscreen_draw_buffer *buffer, po_pixel colour)
{
    po_pixel *cursor = buffer->data;
    po_pixel *end = buffer->data + buffer->width * buffer->height;

    while (cursor != end)
    {
        *cursor++ = colour;
    }
}

// DEBUG @tmp
internal void
draw_lines(struct po_line *lines, size_t line_count, vec2 offset,
        offscreen_draw_buffer *buffer, po_arena *arena)
{
    po_pixel line_colour = {.r = 200, .g = 200, .b = 200};
    for (int i = 0; i < line_count; i++)
    {
        po_line line = lines[i];

        // Move line to screen space
        line.va.x += offset.x;
        line.va.y += offset.y;
        line.vb.x += offset.x;
        line.vb.y += offset.y;

        // Chop up the line if it goes out of bounds
        po_line *segments;
        size_t segment_count;
        if (!(segments = line_divide(line,
                                     0, buffer->width - 1,
                                     0, buffer->height - 1,
                                     arena, &segment_count)))
            // TODO: Log or ?
            return;

        for (size_t i = 0; i < segment_count; i++)
        {
            int32_t Ax = segments[i].va.x;
            int32_t Ay = segments[i].va.y;
            int32_t Bx = segments[i].vb.x;
            int32_t By = segments[i].vb.y;

            int32_t dx = Bx - Ax;
            int32_t dy = By - Ay;

            // TODO: Queue all render jobs and do them at once
            // for all objects, not just the ship
            // Use frame stack allocator

            int32_t steps;
            {
                int32_t abs_dx = ABS(dx);
                int32_t abs_dy = ABS(dy);
                steps = abs_dx > abs_dy ? abs_dx : abs_dy;
            }

            float x_step = (float)dx / steps;
            float y_step = (float)dy / steps;

            float cur_x = Ax;
            float cur_y = Ay;

            for (int i = 0; i < steps; i++) {
                size_t pos = ROUND(cur_y) * buffer->width + ROUND(cur_x);
                ASSERT(pos >= 0);
                ASSERT(pos < buffer->width * buffer->height);
                buffer->data[pos] = line_colour;
                cur_x += x_step;
                cur_y += y_step;
            }
        }
    }
}

/* ========================================================================== */

object_id new_object(vec2 pos, vec2 vel, drawable d, objects_in_space *objects)
{
    ASSERT(objects->count < objects->capacity);

    // TODO: Rework to handle objects dieing and being reused
    object_id id = objects->count;

    objects->positions[id] = pos;
    objects->velocities[id] = vel;
    objects->drawables[id] = d;

    objects->count++;
    return id;
}

void update_ship(game_state *state, game_input *input, float delta_time)
{
    static float rotation_factor = 0.1f;
    static float resistance_factor = 0.01f;

    struct ship *the_ship = state->the_ship;

    the_ship->previous_heading = the_ship->heading;
    if (input->left.is_down) {
        the_ship->heading -= rotation_factor;
        if (the_ship->heading < 0)
            the_ship->heading += TWOPI;
    }
    if (input->right.is_down) {
        the_ship->heading += rotation_factor;
        if (the_ship->heading >= TWOPI)
            the_ship->heading -= TWOPI;
    }

    if (the_ship->heading != the_ship->previous_heading) {
        float delta_heading = the_ship->heading - the_ship->previous_heading;

        // TODO: Do we want to store the rotated lines, or just calculate the
        // rotation when drawing (like we do with position)?
        rotate_lines(state->objects->drawables[the_ship->id].line_count,
                state->objects->drawables[the_ship->id].lines,
                delta_heading);

        the_ship->acceleration = vector_rotate(the_ship->acceleration, delta_heading);
    }

    // TODO: Improve interface
    vec2 *ship_velocity = &state->objects->velocities[state->the_ship->id];

    if (input->thrust.is_down) {
        // NOTE:
        // - The acceleration has a constant magnitude
        // - The velocity vector denotes direction and speed (it's magnitude)
        // TODO: Set a terminal velocity - it's technically susceptible to wrapping
        vec2 velocity_change = vector_multiply_scalar(the_ship->acceleration, delta_time);
        // TODO: Avoid having to negate y everywhere
        velocity_change.y *= -1;
        // TODO: Why do we also have to negate x here?
        velocity_change.x *= -1;
        *ship_velocity = vector_add(*ship_velocity, velocity_change);

    } else {
        // Enforce a gradual decelleration when not under thrust
        if (ship_velocity->x != 0 || ship_velocity->y != 0) {
            vec2 resist =
                vector_multiply_scalar(*ship_velocity, -1 * resistance_factor);
            *ship_velocity = vector_add(*ship_velocity, resist);
        }
    }
}

// TODO: @critical
// Currently the update functions assume that count objects are stored linearly
// in memory, but this will not be the case for long.
// We need a more robust solution

void update_positions(vec2 *positions, vec2 *velocities, u32 count)
{
    for (u32 i = 0; i < count; i++)
    {
        positions[i] = vector_add(velocities[i], positions[i]);
    }
}

void wrap_positions(vec2 *positions, u32 count, offscreen_draw_buffer buffer)
{
    for (u32 i = 0; i < count; i++)
    {
        vec2 pos = positions[i];
        // TODO: There has to be a better approach than this
        if (pos.x < 0) {
            int32_t factor = ABS(pos.x + 0.5) / buffer.width + 1;
            float new_pos = pos.x + buffer.width * factor;
            pos.x = new_pos;
        }
        else if (pos.x >= buffer.width) {
            int32_t factor = ABS(pos.x) / buffer.width;
            float new_pos = pos.x - buffer.width * factor;
            pos.x = new_pos;
        }
        if (pos.y < 0) {
            int32_t factor = ABS(pos.y + 0.5) / buffer.height + 1;
            float new_pos = pos.y + buffer.height * factor;
            pos.y = new_pos;
        }
        else if (pos.y >= buffer.height) {
            int32_t factor = ABS(pos.y) / buffer.height;
            float new_pos = pos.y - buffer.height * factor;
            pos.y = new_pos;
        }
    }
}

void draw_objects(game_state *state, offscreen_draw_buffer *buffer)
{
    // TODO: Refactor to have an ordered array of "active" object_id's
    // This will fail once we start adding, removing, reusing object_id's
    for (size_t i = 0; i < state->objects->count; i++)
    {
        drawable d = state->objects->drawables[i];
        vec2 p = state->objects->positions[i];
        draw_lines(d.lines, d.line_count, p, buffer, &state->temporary_memory);
    }
}

/* ========================================================================== */

GAME_INIT(game_init)
{
    // NOTE: The memory struct gives us the potential to reach back into the
    // platform layer as and when necessary. For now things are still a little
    // rough and ready
    // TODO: Allocate memory from here
    game_state *state = memory->state = (void *)memory->base;

    state->persistent_memory = po_arena_init(
            persistent_storage_size - sizeof(game_state),
            memory->base + sizeof(game_state));
    state->temporary_memory = po_arena_init(
            temporary_storage_size,
            memory->base + persistent_storage_size);

    ship *the_ship = PUSH_STRUCT(ship, &state->persistent_memory);

    asteroid *asteroids = PUSH_ARRAY(asteroid, MAX_ASTEROIDS, &state->persistent_memory);
    size_t asteroid_count = 3;

    // TODO: Pool allocation for common object data
    objects_in_space *objects = PUSH_STRUCT(objects_in_space, &state->persistent_memory);

    // For now we just allocate enough memory for each array to hold all
    // forseeable objects; it's small enough.
    // This strategy is subject to change depending on what directions get taken
    // later; e.g. will we find we have occasional or constant need for a lot
    // more objects?
    *objects = (objects_in_space){
        .count = 0,
        .capacity = MAX_OBJECTS,
        .positions  = PUSH_ARRAY(vec2, MAX_OBJECTS, &state->persistent_memory),
        .velocities = PUSH_ARRAY(vec2, MAX_OBJECTS, &state->persistent_memory),
        .drawables  = PUSH_ARRAY(drawable, MAX_OBJECTS, &state->persistent_memory),
    };

    // TODO: Implement getting a new object_id, etc for ship and asteroids

    *the_ship = (ship){
        .acceleration = {0, THRUST_QTY},
        .id = new_object(
            (vec2){.x = buffer->width / 2.0f, .y = buffer->height / 2.0f},
            (vec2){0, 0},
            (drawable){
                .line_count = 5,
                .lines = PUSH_ARRAY(po_line, 5, &state->persistent_memory)
            },
            objects
        )
    };

    // The lines are in local coordinate space, based on the position
    objects->drawables[the_ship->id].lines[0] = (po_line){{  0, -30}, { 20,  20}};
    objects->drawables[the_ship->id].lines[1] = (po_line){{ 20,  20}, {-20,  20}};
    objects->drawables[the_ship->id].lines[2] = (po_line){{-20,  20}, {  0, -30}};
    objects->drawables[the_ship->id].lines[3] = (po_line){{  0,  10}, {  0, -10}};
    objects->drawables[the_ship->id].lines[4] = (po_line){{-10,   0}, { 10,   0}};

    // DEBUG @tmp
    for (size_t i = 0; i < asteroid_count; i++)
    {
        // TODO:
        // - Randomised position and velocity
        // - Shape generation ?

        asteroids[i] = (asteroid){
            .id = new_object(
                (vec2){0},
                (vec2){.x = ((i + 0xdeadbeef) << 33) % 10, .y = ((i + 0xfeedcafe) << 33) % 10},
                (drawable){
                    .line_count = 10,
                    .lines = PUSH_ARRAY(po_line, 10, &state->persistent_memory)
                },
                objects
            )
        };

        objects->drawables[asteroids[i].id].lines[0] = (po_line){{  0, -17}, { 17, -36}};
        objects->drawables[asteroids[i].id].lines[1] = (po_line){{ 17, -36}, { 36, -17}};
        objects->drawables[asteroids[i].id].lines[2] = (po_line){{ 36, -17}, { 26,   0}};
        objects->drawables[asteroids[i].id].lines[3] = (po_line){{ 26,   0}, { 36,  17}};
        objects->drawables[asteroids[i].id].lines[4] = (po_line){{ 36,  17}, { 10,  36}};
        objects->drawables[asteroids[i].id].lines[5] = (po_line){{ 10,  36}, {-17,  36}};
        objects->drawables[asteroids[i].id].lines[6] = (po_line){{-17,  36}, {-36,  17}};
        objects->drawables[asteroids[i].id].lines[7] = (po_line){{-36,  17}, {-36, -17}};
        objects->drawables[asteroids[i].id].lines[8] = (po_line){{-36, -17}, {-17, -36}};
        objects->drawables[asteroids[i].id].lines[9] = (po_line){{-17, -36}, {  0, -17}};
    }

    state->the_ship = the_ship;
    state->asteroids = asteroids;
    state->asteroid_count = asteroid_count;
    state->objects = objects;

    return 0;
}

GAME_UPDATE_AND_RENDER(game_update_and_render)
{
    game_state *state = memory->state;

    // TODO: Don't just hardcode this; even though we're fixed frame rate?
    static float delta_time = 1.0f / NSTOMS(PULSE);

    po_pixel clear_colour = {30, 30, 30};
    clear_draw_buffer(buffer, clear_colour);

    update_ship(state, input, delta_time);

    // TODO: delta_time should likely be used here as well
    update_positions(state->objects->positions, state->objects->velocities,
            state->objects->count);

    wrap_positions(state->objects->positions, state->objects->count, *buffer);

    draw_objects(state, buffer);

    // Clean up for next frame
    po_arena_clear(&state->temporary_memory);

    return 0;
}

internal void
rotate_lines(size_t n, po_line lines[n], float rad)
{
    for (size_t i = 0; i < n; i++)
    {
        // TODO: This needs to be arranged so it can be done wide;
        // multiple rotations at once
        po_line *line = lines + i;
        *line = (po_line){.va = vector_rotate(line->va, rad),
                          .vb = vector_rotate(line->vb, rad)};
    }
}

/* ========================================================================== */

int
line_stack_push(po_stack *stack, po_line line)
{
    // A rudimentary check, but better than nothing I guess
    ASSERT(stack->member_size == sizeof(line));
    return po_stack_push(stack, &line);
}

/*
 * WARNING:
 * The onus in on the caller to call this with an appropriate stack
 */
po_line *
line_stack_pop(po_stack *stack)
{
    return (po_line *)po_stack_pop(stack);
}

/* ========================================================================== */

typedef enum out_code out_code;
enum out_code {
    INSIDE = 0x0,
    LEFT   = 0x1 << 0,
    RIGHT  = 0x1 << 1,
    BOTTOM = 0x1 << 2,
    TOP    = 0x1 << 3
};

/*
 * Compute the bit code for a point (x, y) using the clip plane bounded
 * diagonally by (xmin, ymin), (xmax, ymax)
 */
internal out_code
compute_out_code(int x, int y, int xmin, int xmax, int ymin, int ymax)
{
    // initialised as being inside of [[clip window]]
    out_code code = INSIDE;

    code |= (x < xmin) * LEFT;
    code |= (x > xmax) * RIGHT;
    code |= (y < ymin) * BOTTOM;
    code |= (y > ymax) * TOP;

    return code;
}

/*
 * Determine if a line crosses the draw bounds and segment as necessary
 *
 * Return:
 * - Array of line segments
 * - Array length is written to out_count
 *
 * TODO: Improve interface
 *
 * Cohen–Sutherland clipping algorithm clips a line from
 * P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with
 * diagonal from (xmin, ymin) to (xmax, ymax).
 * [https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm]
 *
 */
internal po_line *
line_divide(po_line line, int xmin, int xmax, int ymin, int ymax,
        po_arena *arena, size_t *out_count)
{
#define SEG_STACK_SZ 32
#define IN_BOUNDS(i, min, max) ((i) >= (min) && (i) <= (max))

    // Under reasonable game conditions, a line may be divided into up to three
    // segements
    // NOTE: Sufficiently long lines can cause a crash; we believe only by
    // trying to write out of bounds (dependent on SEG_STACK_SIZE)
    // But no reasonable objects in this game should do this
    po_stack segments = CREATE_LINE_STACK(SEG_STACK_SZ, arena);
    po_stack safe_segments = CREATE_LINE_STACK(SEG_STACK_SZ, arena);

    if (!segments.data || !safe_segments.data) {
        LOG_ERROR("Failed to allocate memory for line division");
        return NULL;
    }

    line_stack_push(&segments, line);

    po_line *current;
    out_code out_code0;
    out_code out_code1;

    while ((current = line_stack_pop(&segments)))
    {
        int x0 = current->va.x;
        int y0 = current->va.y;
        int x1 = current->vb.x;
        int y1 = current->vb.y;

        // compute out_codes for P0, P1, and whatever point lies outside the clip rectangle
        out_code0 = compute_out_code(x0, y0, xmin, xmax, ymin, ymax);
        out_code1 = compute_out_code(x1, y1, xmin, xmax, ymin, ymax);

        if ((out_code0 | out_code1) == INSIDE) {
            // Both points are inside
            line_stack_push(&safe_segments, *current);
            continue;

        } else if (out_code0 & out_code1) {
            // Both points are out of bounds and share at least one sector

            // Wrap when segments are both in the same sector
            if (out_code0 == out_code1) {
                po_line wrapped = *current;

                if (out_code0 & TOP) {
                    wrapped.va.y -= ymax;
                    wrapped.vb.y -= ymax;
                } else if (out_code0 & BOTTOM) {
                    wrapped.va.y += ymax;
                    wrapped.vb.y += ymax;
                }

                if (out_code0 & LEFT) {
                    wrapped.va.x += xmax;
                    wrapped.vb.x += xmax;
                } else if (out_code0 & RIGHT) {
                    wrapped.va.x -= xmax;
                    wrapped.vb.x -= xmax;
                }

                // NOTE: Stupid long lines mean we may still need to do more
                // dividing. Realistically though, this game should be able to
                // assume the segment is safe at this point
                line_stack_push(&segments, wrapped);

                continue;
            }

            // The segment is out of bounds and straddling at least one sector
            // boundary
            // We need to divide the segment, but to do this, at least one of
            // its points must be INSIDE
            // To get it back inside we must move it along it's shared sector
            // axis - e.g. both points share TOP, therefore must be moved along
            // the Y-axis
            out_code shared = out_code0 & out_code1;

            switch (shared)
            {
            case TOP:    y0 -= ymax; y1 -= ymax; break;
            case BOTTOM: y0 += ymax; y1 += ymax; break;
            case LEFT:   x0 += xmax; x1 += xmax; break;
            case RIGHT:  x0 -= xmax; x1 -= xmax; break;
            default: break;  // Do nothing
            }

            line_stack_push(&segments, LINE(x0, y0, x1, y1));

            continue;
        }

        // At least one endpoint is outside the clip rectangle; pick it.
        out_code out_code_out = out_code0 != INSIDE ? out_code0 : out_code1;
        int32_t x_intersect = INT32_MAX, y_intersect = INT32_MAX;

        // Now find the intersection point;
        // use formulas:
        //   slope = (y1 - y0) / (x1 - x0)
        //   x_intersect = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
        //   y_intersect = y0 + slope * (xm - x0), where xm is xmin or xmax
        // No need to worry about divide-by-zero because, in each case, the
        // out_code bit being tested guarantees the denominator is non-zero
        // but we should debug ASSERT to be sure
        out_code clip_plane = 0;
        if (out_code_out & TOP) {           // point is above the clip window
            ASSERT((y1 - y0) != 0);
            x_intersect = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
            y_intersect = ymax;
            clip_plane = TOP;
        } else if (out_code_out & BOTTOM) { // point is below the clip window
            ASSERT((y1 - y0) != 0);
            x_intersect = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
            y_intersect = ymin;
            clip_plane = BOTTOM;
        } else if (out_code_out & RIGHT) {  // point is to the right of clip window
            ASSERT((x1 - x0) != 0);
            y_intersect = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
            x_intersect = xmax;
            clip_plane = RIGHT;
        } else if (out_code_out & LEFT) {   // point is to the left of clip window
            ASSERT((x1 - x0) != 0);
            y_intersect = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
            x_intersect = xmin;
            clip_plane = LEFT;
        }

        ASSERT(x_intersect != INT32_MAX);
        ASSERT(y_intersect != INT32_MAX);

        // Divide the line and queue for re-testing
        // TODO: This introduces an off by one error, in conjunction with the
        // fact we pass max-inclusive
        if (out_code_out == out_code0) {
            // Store both segments for further processing
            line_stack_push(&segments, LINE(x0, y0,
                        x_intersect - (1 * clip_plane == LEFT)   + (1 * clip_plane == RIGHT),
                        y_intersect - (1 * clip_plane == BOTTOM) + (1 * clip_plane == TOP)));
            line_stack_push(&segments, LINE(x_intersect, y_intersect, x1, y1));
        } else {
            // Store both segments for further processing
            line_stack_push(&segments, LINE(x1, y1,
                        x_intersect - (1 * clip_plane == LEFT)   + (1 * clip_plane == RIGHT),
                        y_intersect - (1 * clip_plane == BOTTOM) + (1 * clip_plane == TOP)));
            line_stack_push(&segments, LINE(x_intersect, y_intersect, x0, y0));
        }

    }

    // TODO: This is arguably very janky indeed. To change or not to change?
    *out_count = safe_segments.top;
    // Sanity check
    ASSERT(*out_count <= SEG_STACK_SZ);
    return (po_line *)safe_segments.data;
}
