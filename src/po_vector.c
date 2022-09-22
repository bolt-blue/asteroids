#include <math.h>

#include "po_vector.h"

/* ========================================================================== */

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

