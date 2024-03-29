#include "dr_api.h"
#include "drmgr.h"
#include "drreg.h"
#include "drsyscall.h"

#include "drtaint.h"
#include "propagation.h"
#include "shadow_registers.h"
#include "shadow_memory.h"

// TODO: 1. Integrate googletest
// TODO: 2. Integrate dr_log
// TODO: 3. Report api errors
// TODO: 4. Expose drtaint API

static dr_emit_flags_t
event_app_instruction(void *drcontext, void *tag, instrlist_t *ilist, instr_t *where,
                      bool for_trace, bool translating, void *user_data);
static bool
event_pre_syscall(void *drcontext, int sysnum);

static void
event_post_syscall(void *drcontext, int sysnum);

static int drtaint_init_count;
static client_id_t client_id;

/* ======================================================================================
 * Main
 * ==================================================================================== */

#pragma region main

bool
drtaint_init(client_id_t id)
{
    drreg_options_t drreg_ops = { sizeof(drreg_ops), 4, false };
    drsys_options_t drsys_ops = { sizeof(drsys_ops), 0 };
    drmgr_priority_t pri = {
        sizeof(pri), DRMGR_PRIORITY_NAME_DRTAINT,   NULL,
        NULL,        DRMGR_PRIORITY_INSERT_DRTAINT,
    };

    int count = dr_atomic_add32_return_sum(&drtaint_init_count, 1);
    if (count > 1)
        return true;

    drmgr_init();

    if (!smemory_init(id) || !sregs_init() || drreg_init(&drreg_ops) != DRREG_SUCCESS ||
        drsys_init(id, &drsys_ops) != DRMF_SUCCESS) {
        return false;
    }

    drsys_filter_all_syscalls();
    if (!drmgr_register_bb_instrumentation_event(NULL, event_app_instruction, &pri) ||
        !drmgr_register_pre_syscall_event(event_pre_syscall) ||
        !drmgr_register_post_syscall_event(event_post_syscall)) {
        return false;
    }

    return true;
}

void
drtaint_exit(void)
{
    int count = dr_atomic_add32_return_sum(&drtaint_init_count, -1);
    if (count != 0)
        return;

    drmgr_unregister_pre_syscall_event(event_pre_syscall);
    drmgr_unregister_post_syscall_event(event_post_syscall);

    drmgr_exit();
    drreg_exit();
    drsys_exit();
}

static dr_emit_flags_t
event_app_instruction(void *drcontext, void *tag, instrlist_t *ilist, instr_t *where,
                      bool for_trace, bool translating, void *user_data)
{
    propagate(drcontext, ilist, where);

    // struct status;
    // propagate(status);
    // if status.is_succeeded
    // return
    // if status.is_failed
    // dr_log(status.reason)
    // return
    // else propagate_simd(status)

    return DR_EMIT_DEFAULT;
}

#pragma endregion main


#pragma region syscall_handling


/* ======================================================================================
 * System call handling routines
 * ==================================================================================== */

static bool
drsys_iter_cb(drsys_arg_t *arg, void *drcontext)
/*
 *   Set syscall output parameters untainted
 */
{
    if (!arg->valid)
        return true;

    if (arg->pre)
        return true;

    if (arg->mode & DRSYS_PARAM_OUT) {
        // app_pc buffer = (app_pc)arg->start_addr;
        // drtaint_set_app_area_taint(drcontext, (app_pc)buffer, arg->size, 0);
    }

    return true;
}

static bool
event_pre_syscall(void *drcontext, int sysnum)
{
    drmf_status_t status;
    status = drsys_iterate_memargs(drcontext, drsys_iter_cb, drcontext);
    DR_ASSERT(status == DRMF_SUCCESS);
    return true;
}

static void
event_post_syscall(void *drcontext, int sysnum)
{
    dr_syscall_result_info_t info = { sizeof(info) };
    dr_syscall_get_result_ex(drcontext, &info);

    // All syscalls untaint rax
    // drtaint_set_reg_taint(drcontext, DR_REG_, 0u);

    // We only care about tainting if the syscall succeeded.
    if (!info.succeeded)
        return;

    // Clear taint for system calls with an OUT memarg param
    drmf_status_t status = drsys_iterate_memargs(drcontext, drsys_iter_cb, drcontext);

    DR_ASSERT(status == DRMF_SUCCESS);
}

#pragma endregion syscall_handling

/* ======================================================================================
 * Implementation of API
 * ==================================================================================== */

#pragma region api

#pragma endregion api