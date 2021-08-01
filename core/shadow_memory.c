/**
 * \file
 * Shadow memory cpp
 */

#include "dr_api.h"
#include "drmgr.h"
#include "umbra.h"

#include "defines.h"
#include "shadow_memory.h"

static reg_id_t
get_faulting_shadow_reg(void *drcontext, dr_mcontext_t *mc);

static bool
handle_special_shadow_fault(void *drcontext, dr_mcontext_t *raw_mc, app_pc app_shadow);

#ifdef UNIX
static dr_signal_action_t
event_signal_instrumentation(void *drcontext, dr_siginfo_t *info);
#else
static bool
event_signal_instrumetnation(void *drcontext, dr_exception_t *exc);
#endif

static umbra_map_t *umbra_map;

bool
smemory_init(client_id_t id)
{
    umbra_map_options_t umbra_map_ops;
    drmgr_init();

    /* initialize umbra and lazy page handling */
    memset(&umbra_map_ops, 0, sizeof(umbra_map_ops));
    umbra_map_ops.scale = DRTAINT_MAP_SCALE;
    umbra_map_ops.flags = UMBRA_MAP_CREATE_SHADOW_ON_TOUCH |
                          UMBRA_MAP_SHADOW_SHARED_READONLY;

    umbra_map_ops.default_value = 0;
    umbra_map_ops.default_value_size = 1;

    if (umbra_init(id) != DRMF_SUCCESS)
        return false;

    if (umbra_create_mapping(&umbra_map_ops, &umbra_map) != DRMF_SUCCESS)
        return false;

#ifdef UNIX
    drmgr_register_signal_event(event_signal_instrumentation);
#else
    drmgr_register_exception_event(event_signal_instrumetnation);
#endif
    return true;
}

void
smemory_exit(void)
{
    if (umbra_destroy_mapping(umbra_map) != DRMF_SUCCESS)
        DR_ASSERT(false);

#ifdef UNIX
    drmgr_unregister_signal_event(event_signal_instrumentation);
#else
    drmgr_unregister_exception_event(event_signal_instrumetnation);
#endif
    umbra_exit();
    drmgr_exit();
}

static reg_id_t
get_faulting_shadow_reg(void *drcontext, dr_mcontext_t *mc)
{
    instr_t inst;
    reg_id_t reg;

    instr_init(drcontext, &inst);
    decode(drcontext, mc->pc, &inst);

    DR_ASSERT_MSG(opnd_is_base_disp(instr_get_dst(&inst, 0)), "Emulation error");
    reg = opnd_get_base(instr_get_dst(&inst, 0));
    DR_ASSERT_MSG(reg != DR_REG_NULL, "Emulation error");

    instr_free(drcontext, &inst);
    return reg;
}

static bool
handle_special_shadow_fault(void *drcontext, dr_mcontext_t *raw_mc, app_pc app_shadow)
{
    umbra_shadow_memory_type_t shadow_type;
    app_pc app_target;
    reg_id_t reg;

    /* If a fault occured, it is probably because we computed the
     * address of shadow memory which was initialized to a shared
     * readonly shadow block. We allocate a shadow page there and
     * replace the reg value used by the faulting instr.
     */

    /* handle faults from writes to special shadow blocks */
    if (umbra_shadow_memory_is_shared(umbra_map, app_shadow, &shadow_type) !=
        DRMF_SUCCESS) {
        DR_ASSERT(false);
        return true;
    }
    if (shadow_type != UMBRA_SHADOW_MEMORY_TYPE_SHARED)
        return true;

    /* Grab the original app target out of the spill slot so we
     * don't have to compute the app target ourselves (this is
     * difficult).
     */
    app_target = (app_pc)dr_read_saved_reg(drcontext, FAULT_APP_ADDR_SPILL_SLOT);

    /* replace the shared block, and record the new app shadow */
    if (umbra_replace_shared_shadow_memory(umbra_map, app_target, &app_shadow) !=
        DRMF_SUCCESS) {
        DR_ASSERT(false);
        return true;
    }

    /* Replace the faulting register value to reflect the new shadow
     * memory.
     */
    reg = get_faulting_shadow_reg(drcontext, raw_mc);
    reg_set_value(reg, raw_mc, (reg_t)app_shadow);
    return false;
}

#ifdef UNIX
static dr_signal_action_t
event_signal_instrumentation(void *drcontext, dr_siginfo_t *info)
{
    if (info->sig != SIGSEGV && info->sig != SIGBUS)
        return DR_SIGNAL_DELIVER;

    DR_ASSERT(info->raw_mcontext_valid);
    return handle_special_shadow_fault(drcontext, info->raw_mcontext,
                                       info->access_address)
               ? DR_SIGNAL_DELIVER
               : DR_SIGNAL_SUPPRESS;
}
#else
static bool
event_signal_instrumetnation(void *drcontext, dr_exception_t *exc)
{
    if (exc->record->ExceptionCode != STATUS_ACCESS_VIOLATION)
        return true;

    app_pc target = (app_pc)exc->record->ExceptionInformation[1];
    return handle_special_shadow_fault(drcontext, exc->raw_mcontext, target);
}
#endif

bool
smemory_read_shadow(app_pc addr, size_t app_size, size_t shadow_size, void *buf)
{
    drmf_status_t status;
    size_t n_bytes_read = shadow_size;

    status = umbra_read_shadow_memory(umbra_map, addr, app_size, &shadow_size, buf);
    return status == DRMF_SUCCESS && n_bytes_read == shadow_size;
}

bool
smemory_write_shadow(app_pc addr, size_t app_size, size_t shadow_size, void *buf)
{
    drmf_status_t status;
    size_t n_bytes_written = shadow_size;

    status = umbra_read_shadow_memory(umbra_map, addr, app_size, &shadow_size, buf);
    return status == DRMF_SUCCESS && n_bytes_written == shadow_size;
}

bool
smemory_insert_read_shadow_addr(void *drcontext, instrlist_t *ilist, instr_t *where,
                                reg_id_t reg_addr, reg_id_t reg_scratch)
{
    /* Save the app address to a well-known spill slot, so that the fault handler
     * can recover if no shadow memory was installed yet.
     */
    dr_save_reg(drcontext, ilist, where, reg_addr, FAULT_APP_ADDR_SPILL_SLOT);
    drmf_status_t status = umbra_insert_app_to_shadow(drcontext, umbra_map, ilist, where,
                                                      reg_addr, &reg_scratch, 1);

    return status == DRMF_SUCCESS;
}