/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef PO_UTILITY_H
#define PO_UTILITY_H

/* ========================================================================== */

#include <stdint.h>

typedef uint8_t  bool8;
typedef uint32_t bool32;
#define TRUE     (bool32)1
#define FALSE    (bool32)0

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

#define NANOSEC     1000000000L
#define TEN_FPS     (NANOSEC / 10.0f)
#define THIRTY_FPS  (NANOSEC / 30.0f)
#define SIXTY_FPS   (NANOSEC / 60.0f)
#define NSTOMS(n)   ((n) * 0.000001f)

#define KB(v) ((v) * 1024)
#define MB(v) ((v) * 1024 * 1024)
#define GB(v) ((v) * 1024 * 1024 * 1024)

/* ========================================================================== */

#define STRIFY(s) STRINGIFY(s)
#define STRINGIFY(s) #s

#define HEAD(...) HEAD0(__VA_ARGS__, 0)
#define HEAD0(_0, ...) _0
#define TAIL(...) TAIL0(__VA_ARGS__, 0)
#define TAIL0(_0, ...) __VA_ARGS__

#ifndef NDEBUG
#define ASSERT(cond, ...)                                               \
    do {                                                                \
        if (!(cond)) {                                                  \
            fprintf(stderr, "[ASSERT] %s:%s:" STRIFY(__LINE__) " "      \
                    STRIFY((cond))  " " HEAD(__VA_ARGS__) "%.0d" "\n",  \
                    __FILE__, __func__,                                 \
                    TAIL(__VA_ARGS__));                                 \
            *(volatile int *)0 = 0;                                     \
        }                                                               \
    } while (0)
#else
#define ASSERT(...)
#endif

// TODO: Allow logging to file
#ifndef NTRACE
#define LOG(T, ...)                                                     \
        fprintf(stderr, "[" T "] "                                      \
                HEAD(__VA_ARGS__) "%.0d" "\n",                          \
                TAIL(__VA_ARGS__))
#define LOG_DEBUG(...) LOG("DEBUG",   __VA_ARGS__)
#define LOG_ERROR(...) LOG("ERROR",   __VA_ARGS__)
#define LOG_WARN(...)  LOG("WARNING", __VA_ARGS__)
#define LOG_INFO(...)  LOG("INFO",    __VA_ARGS__)
#else
#define LOG_DEBUG(...)
#define LOG_ERROR(...)
#define LOG_WARN(...)
#define LOG_INFO(...)
#endif

/* ========================================================================== */

#include <time.h>

// TODO:
// - Make this cross platform
// - Does this actually get inlined when optimised?
// - Switch to macro version? https://gist.github.com/diabloneo/9619917#gistcomment-3364033
extern inline void
po_timespec_diff(struct timespec *a, struct timespec *b,
        struct timespec *result)
{
    *result = (struct timespec){.tv_sec = a->tv_sec - b->tv_sec,
                                .tv_nsec = a->tv_nsec - b->tv_nsec};
    if (result->tv_nsec < 0) {
        result->tv_sec--;
        result->tv_nsec += NANOSEC;
    }
}

#endif /* PO_UTILITY_H */
