/**
 * \file
 * Shadow registers
 */

#include <stddef.h>

#include "dr_api.h"
#include "drmgr.h"

#include "defines.h"
#include "shadow_registers.h"

typedef struct _per_thread_t {
    // taint_simd_t simd[DR_NUM_SIMD_VECTOR_REGS];
    taint_gpr_t gpr[DR_NUM_GPR_REGS];
} per_thread_t;

static int tls_index;

static void
event_thread_init(void *drcontext);

static void
event_thread_exit(void *drcontext);

bool
sregs_init(void)
{
    drmgr_priority_t exit_priority = { sizeof(exit_priority),
                                       DRMGR_PRIORITY_NAME_DRTAINT_EXIT, NULL, NULL,
                                       DRMGR_PRIORITY_THREAD_EXIT_DRTAINT };

    drmgr_priority_t init_priority = { sizeof(init_priority),
                                       DRMGR_PRIORITY_NAME_DRTAINT_INIT, NULL, NULL,
                                       DRMGR_PRIORITY_THREAD_INIT_DRTAINT };

    drmgr_init();
    drmgr_register_thread_init_event_ex(event_thread_init, &init_priority);
    drmgr_register_thread_exit_event_ex(event_thread_exit, &exit_priority);

    /* initialize tls for per-thread data */
    tls_index = drmgr_register_tls_field();
    if (tls_index == -1)
        return false;

    return true;
}

void
sregs_exit(void)
{
    drmgr_unregister_tls_field(tls_index);
    drmgr_unregister_thread_init_event(event_thread_init);
    drmgr_unregister_thread_exit_event(event_thread_exit);
    drmgr_exit();
}

static void
event_thread_init(void *drcontext)
{
    per_thread_t *data = dr_thread_alloc(drcontext, sizeof(per_thread_t));
    memset(data, 0, sizeof(per_thread_t));
    drmgr_set_tls_field(drcontext, tls_index, data);
}

static void
event_thread_exit(void *drcontext)
{
    per_thread_t *data = drmgr_get_tls_field(drcontext, tls_index);
    dr_thread_free(drcontext, data, sizeof(per_thread_t));
}

bool
sregs_insert_read_shadow_addr(void *drcontext, instrlist_t *ilist, instr_t *where,
                              reg_id_t reg_num, reg_id_t reg_shadow_addr)
{
    DR_ASSERT(reg_num - DR_REG_START_GPR < DR_NUM_GPR_REGS);
    size_t offs = offsetof(per_thread_t, gpr[reg_num - DR_REG_START_GPR]);

    /* Load the per_thread data structure holding
     * the thread-local taint values of each register to %reg_shadow_addr%
     */
    drmgr_insert_read_tls_field(drcontext, tls_index, ilist, where, reg_shadow_addr);

    /* out <- %reg_shadow_addr% = &reg_num_gprs[offs] */
    instrlist_meta_preinsert(
        ilist, where,
        XINST_CREATE_add(drcontext, /* reg_shadow_addr = reg_shadow_addr + offs */
                         opnd_create_reg(reg_shadow_addr), OPND_CREATE_INT8(offs)));
    return true;
}

void
sregs_set_gpr_zero_taint(void *drcontext, reg_id_t reg_num)
{
    DR_ASSERT(reg_num - DR_REG_START_GPR < DR_NUM_GPR_REGS);
    per_thread_t *data = drmgr_get_tls_field(drcontext, tls_index);
    memset(&data->gpr[reg_num], 0, sizeof(taint_gpr_t));
}

void
sregs_set_gpr_taint(void* drcontext, reg_id_t reg_num, taint_gpr_t *reg_taint)
{
    DR_ASSERT(reg_num - DR_REG_START_GPR < DR_NUM_GPR_REGS);
    per_thread_t *data = drmgr_get_tls_field(drcontext, tls_index);
    memcpy(&data->gpr[reg_num], reg_taint, sizeof(taint_gpr_t));
}

void
sregs_set_gpr_non_zero_taint(void *drcontext, reg_id_t reg_num, size_t tbyte)
{

    DR_ASSERT(reg_num - DR_REG_START_GPR < DR_NUM_GPR_REGS);
    DR_ASSERT(tbyte > SHADOW_BYTE_MIN_VALUE && tbyte <= SHADOW_BYTE_MAX_VALUE);
    taint_byte_t t = (taint_byte_t)tbyte;

#ifdef X64
    taint_gpr_t reg_taint = {
        t, t, t, t, //
        t, t, t, t, //
    };
#else
    taint_gpr_t reg_taint = {
        t, t, t, t, //
    };
#endif

    sregs_set_gpr_taint(drcontext, reg_num, &reg_taint);
}

void
sregs_get_gpr_taint(void *drcontext, reg_id_t reg_num, taint_gpr_t *reg_taint)
{
    DR_ASSERT(reg_num - DR_REG_START_GPR < DR_NUM_GPR_REGS);
    per_thread_t *data = drmgr_get_tls_field(drcontext, tls_index);
    memcpy(reg_taint, &data->gpr[reg_num], sizeof(taint_gpr_t));
}

bool
sregs_is_gpr_tainted_ex(void *drcontext, reg_id_t reg_num, taint_gpr_t *reg_taint)
{
    taint_gpr_t reg;
    sregs_get_gpr_taint(drcontext, reg_num, &reg);
    return memcmp(&reg, reg_taint, sizeof(taint_gpr_t)) != 0;
}

bool
sregs_is_gpr_tainted(void *drcontext, reg_id_t reg_num)
{
    taint_gpr_t reg_no_taint = { 0 };
    return sregs_is_gpr_tainted_ex(drcontext, reg_num, &reg_no_taint);
}