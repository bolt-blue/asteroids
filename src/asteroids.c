/*
 * Asteroids
 *
 * A humble clone of Atari's arcade classic.
 *
 */

// TODO: Switch to using shortened typenames, e.g. u8
#include <math.h>
#include <stdint.h>

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

/* ========================================================================== */

typedef struct vector2d vec2;
typedef struct vector2d point2;
struct vector2d {
    float x; float y;
};

typedef struct vector3d vec3;
struct vector3d {
    float x; float y; float z;
};

typedef struct matrix2x2 mat2x2;
struct matrix2x2 {
    vec2 a, b;
};

typedef struct po_line po_line;
struct po_line {
    point2 va;
    point2 vb;
    uint32_t thickness;
};

#define PI 3.14159265358979323846
#define TWOPI (PI * 2)

struct ship {
    // TODO: Move to using quaternions at some point
    vec2 position;
    vec2 acceleration;
    vec2 velocity;
    // theta in radians
    float heading;
    uint32_t line_count;
    po_line *lines;
};

struct ship the_ship;

internal po_line *line_divide(po_line line, int xmin, int xmax, int ymin, int ymax,
        po_arena *arena, size_t *out_count);

void po_memset(void *mem, int c, size_t n)
{
    uint8_t *m = mem;
    while (n--)
        *m++ = c;
}

vec2 vector_add(vec2 a, vec2 b);
vec2 vector_add_scalar(vec2 v, float amount);
vec2 vector_multiply_scalar(vec2 a, float amount);
vec2 vector_rotate(vec2 v, float amount);
vec3 vector_cross(vec2 a, vec2 b);
float vector_dot(vec2 a, vec2 b);

vec2 vector_add(vec2 a, vec2 b)
{
    return (vec2) {a.x + b.x, a.y + b.y};
}
vec2 vector_add_scalar(vec2 v, float amount)
{
    return (vec2){v.x + amount, v.y + amount};
}
vec2 vector_multiply_scalar(vec2 v, float amount)
{
    return (vec2){v.x * amount, v.y * amount};
}
/*
 * Expects amount in radians
 */
vec2 vector_rotate(vec2 v, float amount)
{
    float sin_amount = sin(amount);
    float cos_amount = cos(amount);
    mat2x2 rot = {{cos_amount, -sin_amount},
                  {sin_amount, cos_amount}};
    vec2 Vxa = vector_multiply_scalar((vec2){rot.a.x, rot.b.x}, v.x);
    vec2 Vxb = vector_multiply_scalar((vec2){rot.a.y, rot.b.y}, v.y);
    return vector_add(Vxa, Vxb);
}
/*
 * This is possibly a bit janky
 */
vec3 vector_cross(vec2 a, vec2 b)
{
    float az = 0;
    float bz = -1;
    return (vec3){
        a.y *  bz -  az * b.y,
         az * b.x - a.x *  bz,
        a.x * b.y - a.y * b.x};
}
float vector_dot(vec2 a, vec2 b)
{
    return a.x * b.x + a.y * b.y;
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

        // TODO: This needs to be arranged so it can be done wide;
        // multiple rotations at once
        line = (po_line){.va = vector_rotate(line.va, the_ship.heading),
                         .vb = vector_rotate(line.vb, the_ship.heading),
                         .thickness = line.thickness};

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
    if (input->left.is_down) {
        the_ship.heading -= rotation_factor;
        if (the_ship.heading < 0)
            the_ship.heading += TWOPI;
    }
    if (input->right.is_down) {
        the_ship.heading += rotation_factor;
        if (the_ship.heading > TWOPI)
            the_ship.heading -= TWOPI;
    }
    if (input->thrust.is_down) {
        // TODO: Don't recalculate this every frame if we don't have to
        the_ship.acceleration = vector_rotate(the_ship.acceleration, the_ship.heading);
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

        // Reset acceleration for the next round
        // TODO: Avoid having to do this
        the_ship.acceleration = (vec2){0, thrust_quantity};
    } else {
        // Enforce a gradual decelleration when not under thrust
        if (the_ship.velocity.x != 0 || the_ship.velocity.y != 0) {
            vec2 resist =
                vector_multiply_scalar(the_ship.velocity, -1 * resistance_factor);
            the_ship.velocity = vector_add(the_ship.velocity, resist);
        }
    }

    the_ship.position = vector_add(the_ship.velocity, the_ship.position);

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

/* ========================================================================== */

typedef enum out_code out_code;
enum out_code {
    INSIDE = 0x0,
    LEFT   = 0x1 << 0,
    RIGHT  = 0x1 << 1,
    BOTTOM = 0x1 << 2,
    TOP    = 0x1 << 3
};

// compute_ the bit code for a point (x, y) using the clip
// bounded diagonally by (xmin, ymin), and (xmax, ymax)

// ASSUME THAT xmax, xmin, ymax and ymin are global constants.
internal out_code compute_out_code(int x, int y, int xmin, int xmax, int ymin, int ymax)
{
    // initialised as being inside of [[clip window]]
    out_code code = INSIDE;

    // TODO: Fix TOP/BOTTOM naming; our y-axis positive direction is down screen
    // - Need a clean approach to handling this throughout the code
#if 0
    if (x < xmin)           // to the left of clip window
        code |= LEFT;
    else if (x > xmax)      // to the right of clip window
        code |= RIGHT;
    if (y < ymin)           // below the clip window
        code |= BOTTOM;
    else if (y > ymax)      // above the clip window
        code |= TOP;
#else
    code |= (x < xmin) * LEFT;
    code |= (x > xmax) * RIGHT;
    code |= (y < ymin) * BOTTOM;
    code |= (y > ymax) * TOP;
#endif

    return code;
}

/*
 * Determine if a line crosses the draw bounds and segment as necessary
 *
 * Return:
 * - Array of line segments
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
    int x0 = line.va.x;
    int y0 = line.va.y;
    int x1 = line.vb.x;
    int y1 = line.vb.y;

    // compute out_codes for P0, P1, and whatever point lies outside the clip rectangle
    out_code out_code0 = compute_out_code(x0, y0, xmin, xmax, ymin, ymax);
    out_code out_code1 = compute_out_code(x1, y1, xmin, xmax, ymin, ymax);

    // Under present conditions, a line may become up to three segements
    // NOTE: We believe it's possible to break things with very long lines, but
    // no reasonable objects in this game should do this
    // TODO: Confirm this
    po_line *segments = po_arena_push(3 * sizeof(po_line), arena);
    po_memset(segments, 0, 3 * sizeof(po_line));
    size_t segment_idx = 0;

    while (TRUE) {
        if (!(out_code0 | out_code1)) {
            // Both points are in bounds; trivially exit loop
            break;

        } else if (out_code0 & out_code1) {
            // Both points share an outside zone (LEFT, RIGHT, TOP, or BOTTOM),
            // so both must be out of bounds

            // Only exit early if they're both in the same sector
            if (out_code0 == out_code1) {
                break;
            }

            // We need to divide the segment again, but to do this, at least
            // one of its points must be INSIDE
            // To get it back inside we must move it along it's shared segment
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

            out_code0 = compute_out_code(x0, y0, xmin, xmax, ymin, ymax);
            out_code1 = compute_out_code(x1, y1, xmin, xmax, ymin, ymax);
            continue;
        }
        // failed both tests, so calculate the line segment to clip
        // from an outside point to an intersection with clip edge
        int x_intersect, y_intersect;

        // At least one endpoint is outside the clip rectangle; pick it.
        out_code out_code_out = out_code1 > out_code0 ? out_code1 : out_code0;

        // Now find the intersection point;
        // use formulas:
        //   slope = (y1 - y0) / (x1 - x0)
        //   x_intersect = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
        //   y_intersect = y0 + slope * (xm - x0), where xm is xmin or xmax
        // No need to worry about divide-by-zero because, in each case, the
        // out_code bit being tested guarantees the denominator is non-zero
        if (out_code_out & TOP) {           // point is above the clip window
            x_intersect = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
            y_intersect = ymax;
        } else if (out_code_out & BOTTOM) { // point is below the clip window
            x_intersect = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
            y_intersect = ymin;
        } else if (out_code_out & RIGHT) {  // point is to the right of clip window
            y_intersect = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
            x_intersect = xmax;
        } else if (out_code_out & LEFT) {   // point is to the left of clip window
            y_intersect = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
            x_intersect = xmin;
        }

#define LINE(x0, y0, x1, y1) (po_line){.va.x = (x0), .va.y = (y0), .vb.x = (x1), .vb.y = (y1)}

        // Then clip and re-test the original line
        // TODO: We probably don't want to handle the screen wrapping here
        if (out_code_out == out_code0) {
            // Point 0 of the line is out of bounds

            // Store the out of bound segment
            segments[segment_idx] = LINE(x0, y0, x_intersect, y_intersect);
            // Move outside point to intersection point to clip
            x0 = x_intersect;
            y0 = y_intersect;
            // Get ready for next pass
            out_code0 = compute_out_code(x0, y0, xmin, xmax, ymin, ymax);

        } else {
            // Point 1 of the line is out of bounds

            // Store the out of bound segment
            segments[segment_idx] = LINE(x1, y1, x_intersect, y_intersect);
            // Move outside point to intersection point to clip
            x1 = x_intersect;
            y1 = y_intersect;
            // Get ready for next pass
            out_code1 = compute_out_code(x1, y1, xmin, xmax, ymin, ymax);
        }

        // Wrap out of bound segments
        if (out_code_out & TOP) {
            segments[segment_idx].va.y -= ymax;
            segments[segment_idx].vb.y -= ymax;
        } else if (out_code_out & BOTTOM) {
            segments[segment_idx].va.y += ymax;
            segments[segment_idx].vb.y += ymax;
        }

        if (out_code_out & LEFT) {
            segments[segment_idx].va.x += xmax;
            segments[segment_idx].vb.x += xmax;
        } else if (out_code_out & RIGHT) {
            segments[segment_idx].va.x -= xmax;
            segments[segment_idx].vb.x -= xmax;
        }

        segment_idx++;
    }

    // The segment here is either fully inside or outside the boundaries
    if (out_code0 == INSIDE) {
        segments[segment_idx] =
            (po_line){ .va.x = x0, .va.y = y0, .vb.x = x1, .vb.y = y1};
    } else {
        // If the out of bounds line is not in a single sector, we need to
        // first divide it again
        if (out_code0 != out_code1) {

        }
        switch (out_code0)
        {
        case TOP: {
            segments[segment_idx++] = LINE(x0, y0 - ymax, x1, y1 - ymax);
        } break;

        case BOTTOM: {
            segments[segment_idx++] = LINE(x0, y0 + ymax, x1, y1 + ymax);
        } break;

        case LEFT: {
            segments[segment_idx++] = LINE(x0 + xmax, y0, x1 + xmax, y1);
        } break;

        case RIGHT: {
            segments[segment_idx++] = LINE(x0 - xmax, y0, x1 - xmax, y1);
        } break;

        case INSIDE: {
            // Do nothing
        } break;
        }
    }


    *out_count = segment_idx + 1;
    return segments;
}
