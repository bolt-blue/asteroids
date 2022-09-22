/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef VECTOR_H
#define VECTOR_H

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

/* ========================================================================== */

vec2 vector_add(vec2 a, vec2 b);
vec2 vector_add_scalar(vec2 v, float amount);
vec2 vector_multiply_scalar(vec2 a, float amount);
vec2 vector_rotate(vec2 v, float amount);
vec3 vector_cross(vec2 a, vec2 b);
float vector_dot(vec2 a, vec2 b);

#endif /* VECTOR_H */
