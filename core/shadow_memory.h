/**
 * \file
 * Shadow memory h
 */

#pragma once
#include "dr_api.h"
// #include "stdbool.h"

#define FAULT_APP_ADDR_SPILL_SLOT SPILL_SLOT_2

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Turn off shadow memory module
 */
bool
smemory_init(client_id_t id);

/**
 * Turn on shadow memory module
 */
void
smemory_exit(void);

/**
 * Read shadow memory
 */
bool
smemory_read_shadow(app_pc addr, size_t app_size, size_t shadow_size, void *buf);

/**
 * Write shadow memory
 */
bool
smemory_write_shadow(app_pc addr, size_t app_size, size_t shadow_size, void *buf);

/**
 *    Save original application address in %reg_addr% to SPILL_SLOT_2
 *    and translate value of %reg_addr% to its shadow address
 */
bool
smemory_insert_read_shadow_addr(void *drcontext, instrlist_t *ilist, instr_t *where,
                                reg_id_t reg_addr, reg_id_t reg_scratch);

#ifdef __cplusplus
}
#endif