/**
 * \file
 * Shadow blocks h
 */

#pragma once
#include "defines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*sblocks_iter_read_byte_func_t)(uint8_t *addr, taint_byte_t tbyte,
                                              void *user_data);

typedef void (*sblocks_iter_write_byte_func_t)(uint8_t *addr, size_t *tbyte,
                                               void *user_data);

bool
sblocks_is_app_zero_tainted(uint8_t *addr, size_t app_size);

bool
sblocks_is_app_non_zero_tainted(uint8_t *addr, size_t app_size);

bool
sblocks_is_app_non_zero_tainted_ex(uint8_t *addr, size_t size, size_t tbyte);

void
sblocks_iterate_app_taint_read(uint8_t *addr, size_t size,
                               sblocks_iter_read_byte_func_t iterate_func,
                               void *user_data);

void
sblocks_set_app_non_zero_taint(uint8_t *addr, size_t size, size_t tbyte);

void
sblocks_set_app_zero_taint(uint8_t *addr, size_t size);

void
sblocks_iterate_app_taint_write(uint8_t *addr, size_t size,
                                sblocks_iter_write_byte_func_t iterate_func,
                                void *user_data);

// void
// sblocks_copy_app_taint(uint8_t *addr_src, uint8_t *addr_dst, size_t size);

#ifdef __cplusplus
}
#endif