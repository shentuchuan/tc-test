#ifndef __UTIL_PUBLIC_H
#define __UTIL_PUBLIC_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <arpa/inet.h>

typedef signed char   s8;
typedef unsigned char u8;
// typedef unsigned char uint8_t;

typedef signed short   s16;
typedef unsigned short u16;
// typedef unsigned short uint16_t;

typedef signed int   s32;
typedef unsigned int u32;
// typedef unsigned int uint32_t;

typedef signed long long   s64;
typedef unsigned long long u64;

// typedef unsigned long long uint64_t;

/**
 * reciprocal_scale - "scale" a value into range [0, ep_ro)
 * @val: value
 * @ep_ro: right open interval endpoint
 *
 * Perform a "reciprocal multiplication" in order to "scale" a value into
 * range [0, ep_ro), where the upper interval endpoint is right-open.
 * This is useful, e.g. for accessing a index of an array containing
 * ep_ro elements, for example. Think of it as sort of modulus, only that
 * the result isn't that of modulo. ;) Note that if initial input is a
 * small value, then result will return 0.
 *
 * Return: a result based on val in interval [0, ep_ro).
 */
static inline u32 reciprocal_scale(u32 val, u32 ep_ro) {
    return (u32)(((u64)val * ep_ro) >> 32);
}

// #define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#ifndef container_of
#define container_of(ptr, type, member)                    \
    ({                                                     \
        const typeof(((type *)0)->member) *__mptr = (ptr); \
        (type *)((char *)__mptr - offsetof(type, member)); \
    })
#endif

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#ifndef UTIL_MEMCMP
#define UTIL_MEMCMP(x, y, z) memcpy(x, y, z)
#endif

#ifndef UTIL_MALLOC
#define UTIL_MALLOC(x) malloc(x)
#endif

#ifndef UTIL_FREE
#define UTIL_FREE(x)     \
    do {                 \
        if (x != NULL) { \
            free(x);     \
            x = NULL;    \
        }                \
    } while (0)

#endif

#ifndef util_log_error
#define util_log_error(...) fprintf(stderr, __VA_ARGS__)
#endif

#ifndef util_log_warn
#define util_log_warn(...) fprintf(stderr, __VA_ARGS__)
#endif

#ifndef util_log_debug
#define util_log_debug(...) fprintf(stdout, __VA_ARGS__)
#endif

static inline long tv_diff_usec(struct timeval *tv1, struct timeval *tv2) {
    long usec = 0, sec = 0;

    usec = tv1->tv_usec - tv2->tv_usec;
    sec  = tv1->tv_sec - tv2->tv_sec;

    return ((sec * 1000000 + usec));
}

static inline int tv_diff(long tv1, long tv2) {
    return (tv1 - tv2);
}

#endif
