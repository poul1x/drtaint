/**
 * \file
 * Base defines of sregs project
 */

#pragma once
#include "defines.h"
// #include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represents a taint value of general purpose register.
 * Each shadow byte in register must be in range of
 * [#SHADOW_BYTE_MIN_VALUE, #SHADOW_BYTE_MAX_VALUE]
 */
typedef struct _taint_gpr_t {
    taint_byte_t byte1 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte2 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte3 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte4 : N_BITS_PER_SHADOW_BYTE;
#ifdef X64
    taint_byte_t byte5 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte6 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte7 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte8 : N_BITS_PER_SHADOW_BYTE;
#endif
} taint_gpr_t;

/**
 * Represents a taint value of simd register
 * (Neon on ARM32, ARM64 and SSE/AVX on X86, X86_64)
 * Each shadow byte in register must be in range of
 * [#SHADOW_BYTE_MIN_VALUE, #SHADOW_BYTE_MAX_VALUE]
 */
typedef struct _taint_simd_t {
    taint_byte_t byte1 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte2 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte3 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte4 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte5 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte6 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte7 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte8 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte9 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte10 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte11 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte12 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte13 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte14 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte15 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte16 : N_BITS_PER_SHADOW_BYTE;
#if defined(X86) || defined(X64)
    taint_byte_t byte17 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte18 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte19 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte20 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte21 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte22 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte23 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte24 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte25 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte26 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte27 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte28 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte29 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte30 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte31 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte32 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte33 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte34 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte35 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte36 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte37 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte38 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte39 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte40 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte41 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte42 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte43 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte44 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte45 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte46 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte47 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte48 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte49 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte50 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte51 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte52 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte53 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte54 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte55 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte56 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte57 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte58 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte59 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte60 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte61 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte62 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte63 : N_BITS_PER_SHADOW_BYTE;
    taint_byte_t byte64 : N_BITS_PER_SHADOW_BYTE;
#endif
} taint_simd_t;

/**
 * Turn on shadow registers module
 */
bool
sregs_init(void);

/**
 * Turn off shadow registers module
 */
void
sregs_exit(void);

/**
 * Inserts instructions to gain shadow %shadow% register's address
 * of the current thread and place result to register of %regaddr%
 */
bool
sregs_insert_read_shadow_addr(void *drcontext, instrlist_t *ilist, instr_t *where,
                              reg_id_t reg_num, reg_id_t reg_shadow_addr);

/**
 * Make each byte of general purpose register with number \p reg_num untainted (*taint*=0)
 *
 * \param[in] reg_num Number of register to taint. Must be in range of
 * [[DR_REG_START_GPR](https://dynamorio.org/dynamorio_docs/dr__ir__opnd_8h.html#a4ca9d6df48c16d2956a306b382034611),
 * DR_REG_STOP_GPR(https://dynamorio.org/dynamorio_docs/dr__ir__opnd_8h.html#a24ceac3fd7c6c0d0d3bda89426e27f3a)]
 */
void
sregs_set_gpr_zero_taint(void *drcontext, reg_id_t reg_num);

/**
 * Make each byte of simd register with number \p reg_num untainted (*taint*=0)
 *
 * \param[in] reg_num Number of register to taint. Must be in range of
 * [DR_REG_Q0, DR_REG_Q15] on ARM32, [DR_REG_Z0, DR_REG_Z31] on ARM64, [DR_REG_START_ZMM,
 * DR_REG_STOP_ZMM] on X86 and X86_64
 */
void
sregs_set_simd_zero_taint(void *drcontext, reg_id_t reg_num);

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
sregs_set_gpr_non_zero_taint(void *drcontext, reg_id_t reg_num, size_t tbyte);

void
sregs_set_gpr_taint(void *drcontext, reg_id_t reg_num, taint_gpr_t *reg_taint);

void
sregs_get_gpr_taint(void *drcontext, reg_id_t reg_num, taint_gpr_t *reg_taint);

bool
sregs_is_gpr_tainted_ex(void *drcontext, reg_id_t reg_num, taint_gpr_t *reg_taint);

bool
sregs_is_gpr_tainted(void *drcontext, reg_id_t reg_num);

#ifdef __cplusplus
}
#endif