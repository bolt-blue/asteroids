/*
 * Asteroids
 *
 * A humble clone of Atari's arcade classic.
 *
 */

// TODO: Switch to using shortened typenames, e.g. u8
#include <stdint.h>
#ifndef NDEBUG
#include <stdio.h>  // For our ASSERT macro only
#endif

// Dirty unity build
// TODO: Do incremental compile and link instead ?
#include "po_utility.h"
#include "po_window.h"

/* ========================================================================== */

#define PULSE THIRTY_FPS

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

// DEBUG @tmp
global struct ship the_ship;

void turn_the_ship(size_t n, po_line lines[n], float rad);

internal po_line *line_divide(po_line line, int xmin, int xmax, int ymin, int ymax,
        po_arena *arena, size_t *out_count);

void po_memset(void *mem, int c, size_t n)
{
    uint8_t *m = mem;
    while (n--) *m++ = c;
}

/* ========================================================================== */

void
clear_screen(po_surface *surface)
{
    po_pixel colour = {30, 30, 30};

    po_pixel *cursor = surface->data;
    po_pixel *end = surface->data + surface->width * surface->height;

    while (cursor != end)
    {
        *cursor++ = colour;
    }
}

// DEBUG @tmp
void
draw_ship(po_surface *surface, po_arena *arena)
{
    po_pixel ship_colour = {.r = 200, .g = 200, .b = 200};
    for (int i = 0; i < the_ship.line_count; i++)
    {
        po_line line = the_ship.lines[i];

        // Move line to screen space
        line.va.x += the_ship.position.x;
        line.va.y += the_ship.position.y;
        line.vb.x += the_ship.position.x;
        line.vb.y += the_ship.position.y;

        // Chop up the line if it goes out of bounds
        po_line *segments;
        size_t segment_count;
        if (!(segments = line_divide(line, 0,
                                     surface->width - 1,
                                     0, surface->height - 1,
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
                ASSERT(ROUND(cur_y) * surface->width + ROUND(cur_x) >= 0);
                ASSERT(ROUND(cur_y) * surface->width + ROUND(cur_x) <
                        surface->width * surface->height);
                surface->data[ROUND(cur_y) * surface->width + ROUND(cur_x)] = ship_colour;
                cur_x += x_step;
                cur_y += y_step;
            }
        }
    }
}

int
game_update_and_render(po_context *context, game_input *input)
{
    static float thrust_quantity = 10;
    static float rotation_factor = 0.1f;
    static float resistance_factor = 0.01f;
    // TODO: Don't just hardcode this; even though we're fixed frame rate?
    static float delta_time = 1.0f / NSTOMS(PULSE);

    po_window *window = context->window;

    // DEBUG @tmp
    static int first_time = 1;
    if (first_time) {
        the_ship = (struct ship){.line_count = 5,
            .lines = po_arena_push(5 * sizeof(po_line), &context->game_memory.persistent_memory),
            .position = {.x = window->width / 2.0f, .y = window->height / 2.0f},
            .velocity = {0, 0}, .acceleration = {0, thrust_quantity}
        };
        // The lines are in local coordinate space, based on the position
        the_ship.lines[0] = (po_line){{  0, -30}, { 20,  20}};
        the_ship.lines[1] = (po_line){{ 20,  20}, {-20,  20}};
        the_ship.lines[2] = (po_line){{-20,  20}, {  0, -30}};
        the_ship.lines[3] = (po_line){{  0,  10}, {  0, -10}};
        the_ship.lines[4] = (po_line){{-10,   0}, { 10,   0}};
        first_time = 0;
    }

    clear_screen(window->surface);

    // Move ship
    // TODO: Separate out once things are working nicely
    the_ship.previous_heading = the_ship.heading;
    if (input->left.is_down) {
        the_ship.heading -= rotation_factor;
        if (the_ship.heading < 0)
            the_ship.heading += TWOPI;
    }
    if (input->right.is_down) {
        the_ship.heading += rotation_factor;
        if (the_ship.heading >= TWOPI)
            the_ship.heading -= TWOPI;
    }

    if (the_ship.heading != the_ship.previous_heading) {
        float delta_heading = the_ship.heading - the_ship.previous_heading;

        turn_the_ship(the_ship.line_count, the_ship.lines, delta_heading);
        the_ship.acceleration = vector_rotate(the_ship.acceleration, delta_heading);
    }

    if (input->thrust.is_down) {
        // NOTE:
        // - The acceleration has a constant magnitude
        // - The velocity vector denotes direction and speed (it's magnitude)
        // TODO: Set a terminal velocity - it's technically susceptible to wrapping
        vec2 velocity_change = vector_multiply_scalar(the_ship.acceleration, delta_time);
        // TODO: Avoid having to negate y everywhere
        velocity_change.y *= -1;
        // TODO: Why do we also have to negate x here?
        velocity_change.x *= -1;
        the_ship.velocity = vector_add(the_ship.velocity, velocity_change);

    } else {
        // Enforce a gradual decelleration when not under thrust
        if (the_ship.velocity.x != 0 || the_ship.velocity.y != 0) {
            vec2 resist =
                vector_multiply_scalar(the_ship.velocity, -1 * resistance_factor);
            the_ship.velocity = vector_add(the_ship.velocity, resist);
        }
    }

    the_ship.position = vector_add(the_ship.velocity, the_ship.position);

    // TODO: Pull this out as soon as we have more objects
    if (the_ship.position.x < 0) {
        int32_t factor = ABS(the_ship.position.x + 0.5) / window->surface->width + 1;
        float new_pos = the_ship.position.x + window->surface->width * factor;
        the_ship.position.x = new_pos;
    }
    else if (the_ship.position.x >= window->surface->width) {
        int32_t factor = ABS(the_ship.position.x) / window->surface->width;
        float new_pos = the_ship.position.x - window->surface->width * factor;
        the_ship.position.x = new_pos;
    }
    if (the_ship.position.y < 0) {
        int32_t factor = ABS(the_ship.position.y + 0.5) / window->surface->height + 1;
        float new_pos = the_ship.position.y + window->surface->height * factor;
        the_ship.position.y = new_pos;
    }
    else if (the_ship.position.y >= window->surface->height) {
        int32_t factor = ABS(the_ship.position.y) / window->surface->height;
        float new_pos = the_ship.position.y - window->surface->height * factor;
        the_ship.position.y = new_pos;
    }

    draw_ship(window->surface, &context->game_memory.temporary_memory);

    // Clean up for next frame
    po_arena_clear(&context->game_memory.temporary_memory);

    return 0;
}

void turn_the_ship(size_t n, po_line lines[n], float rad)
{
    for (size_t i = 0; i < n; i++)
    {
        // TODO: This needs to be arranged so it can be done wide;
        // multiple rotations at once
        po_line *line = lines + i;
        *line = (po_line){.va = vector_rotate(line->va, rad),
                          .vb = vector_rotate(line->vb, rad),
                          .thickness = line->thickness};
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
po_line *
line_divide(po_line line, int xmin, int xmax, int ymin, int ymax,
        po_arena *arena, size_t *out_count)
{
#define SEG_STACK_SZ 3
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
