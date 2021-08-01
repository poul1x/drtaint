#pragma once

#include "dr_defines.h"
#include "../defines.h"
#include "../shadow_registers.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ======================================================================================
 * Init / exit
 * ==================================================================================== */

/**
 * Turn on drtaint module
 */
bool
drtaint_init(client_id_t id);

/**
 * Turn off drtaint module
 */
void
drtaint_exit(void);

/* ======================================================================================
 * Registers
 * ==================================================================================== */

/**
 * Make each byte of general purpose register with number \p reg_num untainted (*taint*=0)
 *
 * \param[in] reg_num Number of register to taint. Must be in range of
 * [[DR_REG_START_GPR](https://dynamorio.org/dynamorio_docs/dr__ir__opnd_8h.html#a4ca9d6df48c16d2956a306b382034611),
 * DR_REG_STOP_GPR(https://dynamorio.org/dynamorio_docs/dr__ir__opnd_8h.html#a24ceac3fd7c6c0d0d3bda89426e27f3a)]
 */
void
drtaint_set_gpr_zero_taint(void *drcontext, reg_id_t reg_num);

/**
 * Make each byte of simd register with number \p reg_num untainted (*taint*=0)
 *
 * \param[in] reg_num Number of register to taint. Must be in range of
 * [DR_REG_Q0, DR_REG_Q15] on ARM32, [DR_REG_Z0, DR_REG_Z31] on ARM64, [DR_REG_START_ZMM,
 * DR_REG_STOP_ZMM] on X86 and X86_64
 */
void
drtaint_set_simd_zero_taint(void *drcontext, reg_id_t reg_num);

/**
 * Make each byte of simd register with number \p reg_num tainted (*taint*>0)
 *
 * \param[in] reg_num Number of register to taint. Must be in range of
 * [[DR_REG_START_GPR](https://dynamorio.org/dynamorio_docs/dr__ir__opnd_8h.html#a4ca9d6df48c16d2956a306b382034611),
 * DR_REG_STOP_GPR(https://dynamorio.org/dynamorio_docs/dr__ir__opnd_8h.html#a24ceac3fd7c6c0d0d3bda89426e27f3a)]
 * \param[in] tbyte Taint value will be set to each shadow byte of registry. Must be in range of
 * [#SHADOW_BYTE_MIN_VALUE, #SHADOW_BYTE_MAX_VALUE]
 */
void
drtaint_set_gpr_non_zero_taint(void *drcontext, reg_id_t reg_num, size_t tbyte);

void
drtaint_set_gpr_taint(void *drcontext, reg_id_t reg_num, taint_gpr_t *reg_taint);

void
drtaint_get_gpr_taint(void *drcontext, reg_id_t reg_num, taint_gpr_t *reg_taint);

bool
drtaint_is_gpr_tainted_ex(void *drcontext, reg_id_t reg_num, taint_gpr_t *reg_taint);

bool
drtaint_is_gpr_tainted(void *drcontext, reg_id_t reg_num);

/* ======================================================================================
 * Memory
 * ==================================================================================== */

bool
drtaint_is_app_zero_tainted(uint8_t *addr, size_t app_size);

bool
drtaint_is_app_non_zero_tainted(uint8_t *addr, size_t app_size);

bool
drtaint_is_app_non_zero_tainted_ex(uint8_t *addr, size_t size, size_t tbyte);

void
drtaint_iterate_app_taint_read(uint8_t *addr, size_t size,
                               bool(*f)(uint8_t *addr, taint_byte_t tbyte, void *user_data),
                               void *user_data);

void
drtaint_set_app_non_zero_taint(uint8_t *addr, size_t size, size_t tbyte);

void
drtaint_set_app_zero_taint(uint8_t *addr, size_t size);

void
drtaint_iterate_app_taint_write(uint8_t *addr, size_t size,
                                void(*f)(uint8_t *addr, size_t *tbyte, void *user_data),
                                void *user_data);

/* ======================================================================================
 * High level
 * ==================================================================================== */

// drtaint_add_argv_to_taint
// drtaint_remove_argv_to_taint
// drtaint_add_file_to_taint
// drtaint_remove_file_to_taint
// drtaint_add_env_to_taint
// drtaint_remove_env_to_taint
// drtaint_add_network_connection_to_taint
// drtaint_remove_network_connection_to_taint

/* ======================================================================================
 * Callbacks
 * ==================================================================================== */

// drtaint_register_tainted_instr_event
// drtaint_register_before_instrumentation_event
// drtaint_register_after_instrumentation_event
// drtaint_unregister_tainted_instr_event
// drtaint_unregister_before_instrumentation_event
// drtaint_unregister_after_instrumentation_event

/* ======================================================================================
 * Advanced usage
 * ==================================================================================== */

// drtaint_enable_for_this_thread();
// drtaint_disable_for_this_thread();
// drtaint_enable_for_all_threads();
// drtaint_disable_for_all_threads();
// drtaint_include_module()
// drtaint_exclude_module()

#ifdef __cplusplus
}
#endif