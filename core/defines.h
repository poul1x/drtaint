/**
 * \file
 * Base defines of drtaint project
 */

#pragma once
#include <stdint.h>

#define DRMGR_PRIORITY_NAME_DRTAINT "drtaint"
#define DRMGR_PRIORITY_NAME_DRTAINT_EXIT "drtaint.exit"
#define DRMGR_PRIORITY_NAME_DRTAINT_INIT "drtaint.init"

#define DRMGR_PRIORITY_INSERT_DRTAINT -7500
#define DRMGR_PRIORITY_THREAD_INIT_DRTAINT -7500
#define DRMGR_PRIORITY_THREAD_EXIT_DRTAINT 7500

#define DRTAINT_MAP_SCALE DRTAINT_MAP_SCALE_UP_2X

/**
 * Shadow memory mapping (scaling) schemes supported by drtaint. Must be set
 * manualy with `-DDRTAINT_MAP_SCALE=XXX`. Can be: #DRTAINT_MAP_SCALE_DOWN_8X,
 * #DRTAINT_MAP_SCALE_DOWN_4X, #DRTAINT_MAP_SCALE_DOWN_2X, #DRTAINT_MAP_SCALE_SAME_1X,
 * #DRTAINT_MAP_SCALE_UP_2X. See
 * [UMBRA_MAP_SCALE](https://dynamorio.org/drmemory_docs/group__umbra.html#ga670274c9cd812c91317125c7179cb15a)
 */
#ifndef DRTAINT_MAP_SCALE
    #define DRTAINT_MAP_SCALE -1
#endif

/**
 * Shadow memory mapping, where 8 app bytes = 1 shadow byte
 * \warning 64 bit only
 */
#define DRTAINT_MAP_SCALE_DOWN_8X 0

/** Shadow memory mapping, where 4 app bytes = 1 shadow byte */
#define DRTAINT_MAP_SCALE_DOWN_4X 1

/** Shadow memory mapping, where 2 app bytes = 1 shadow byte */
#define DRTAINT_MAP_SCALE_DOWN_2X 2

/** Shadow memory mapping, where 1 app byte = 1 shadow byte */
#define DRTAINT_MAP_SCALE_SAME_1X 3

/** Shadow memory mapping, where 1 app byte = 2 shadow bytes */
#define DRTAINT_MAP_SCALE_UP_2X 4


/**
 * Count of shadow memory bits, mapped to
 * one real byte of application memory.
 * This value depends on #DRTAINT_MAP_SCALE
 */
#if DRTAINT_MAP_SCALE == DRTAINT_MAP_SCALE_DOWN_8X
    #define N_BITS_PER_SHADOW_BYTE 1
#elif DRTAINT_MAP_SCALE == DRTAINT_MAP_SCALE_DOWN_4X
    #define N_BITS_PER_SHADOW_BYTE 2
#elif DRTAINT_MAP_SCALE == DRTAINT_MAP_SCALE_DOWN_2X
    #define N_BITS_PER_SHADOW_BYTE 4
#elif DRTAINT_MAP_SCALE == DRTAINT_MAP_SCALE_SAME_1X
    #define N_BITS_PER_SHADOW_BYTE 8
#elif DRTAINT_MAP_SCALE == DRTAINT_MAP_SCALE_UP_2X
    #define N_BITS_PER_SHADOW_BYTE 16
#else
    #define N_BITS_PER_SHADOW_BYTE -1
#endif

#if N_BITS_PER_SHADOW_BYTE == -1
    #error Invalid DRTAINT_MAP_SCALE. Must be DRTAINT_MAP_SCALE_DOWN_8X, DRTAINT_MAP_SCALE_DOWN_4X, DRTAINT_MAP_SCALE_DOWN_2X, DRTAINT_MAP_SCALE_SAME_1X or DRTAINT_MAP_SCALE_UP_2X
#endif

// #if !defined(X64) && DRTAINT_MAP_SCALE == DRTAINT_MAP_SCALE_DOWN_8X
//     #error Invalid DRTAINT_MAP_SCALE. Must be DRTAINT_MAP_SCALE_DOWN_4X, DRTAINT_MAP_SCALE_DOWN_2X, DRTAINT_MAP_SCALE_SAME_1X or DRTAINT_MAP_SCALE_UP_2X
// #endif

/**
 * Count of bits in byte
 */
#define N_BITS_PER_BYTE 8

/**
 * Indicates that count of bits per shadow byte
 * and count of bits per real byte are equal
 */
#define SHADOW_IS_SCALE_SAME (N_BITS_PER_SHADOW_BYTE == N_BITS_PER_BYTE)

/**
 * Indicates that count of bits per shadow byte
 * is less than count of bits per real byte
 */
#define SHADOW_IS_SCALE_DOWN (N_BITS_PER_SHADOW_BYTE < N_BITS_PER_BYTE)

/**
 * Indicates that count of bits per shadow byte
 * is more than count of bits per real byte
 */
#define SHADOW_IS_SCALE_UP (N_BITS_PER_SHADOW_BYTE > N_BITS_PER_BYTE)

/**
 * Minimum possible value of shadow byte
 */
#define SHADOW_BYTE_MIN_VALUE ((size_t)0)

/**
 * Maximum possible value of shadow byte
 */
#define SHADOW_BYTE_MAX_VALUE ((size_t)((2 << (N_BITS_PER_SHADOW_BYTE - 1)) - 1))

/**
 * Represents type of taint byte.
 * It must have enough space to hold taint value, which depends on shadow scale.
 * See #N_BITS_PER_SHADOW_BYTE for additional information.
 */
#if SHADOW_IS_SCALE_UP
typedef uint16_t taint_byte_t;
#else
typedef uint8_t taint_byte_t;
#endif
