/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef PO_TYPES_H
#define PO_TYPES_H

#include <stddef.h>  // size_t
#include <stdint.h>

/* ========================================================================== */

typedef uint8_t  bool8;
typedef uint32_t bool32;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

#define internal static
#define global static

/* ========================================================================== */

#define TRUE     (bool32)1
#define FALSE    (bool32)0

#define KB(v) ((v) * 1024)
#define MB(v) ((v) * 1024 * 1024)
#define GB(v) ((v) * 1024L * 1024 * 1024)
#define TB(v) ((v) * 1024L * 1024 * 1024 * 1024)

#endif /* PO_TYPES_H */
