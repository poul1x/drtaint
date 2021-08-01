#include "drtaint_helper.h"
#include "drmgr.h"

ReservedRegister::ReservedRegister(void *drcontext, instrlist_t *ilist, instr_t *where)
    : drcontext(drcontext), ilist(ilist), where(where)
{
    bool status = drreg_reserve_register(drcontext, ilist, where, NULL, &reg);
    DR_ASSERT(status == DRREG_SUCCESS);
}

void ReservedRegister::unreserve()
{
    if (reg != DR_REG_NULL)
    {
        bool status = drreg_unreserve_register(drcontext, ilist, where, reg);
        DR_ASSERT(status == DRREG_SUCCESS);
        reg = DR_REG_NULL;
    }
}

ReservedRegister::ReservedRegister(ReservedRegister &&other)
{
    this->drcontext = other.drcontext;
    this->ilist = other.ilist;
    this->where = other.where;
    this->reg = other.reg;
    other.reg = DR_REG_NULL;
}

ReservedRegister::~ReservedRegister()
{
    this->unreserve();
}

DecodedInstruction::DecodedInstruction(void *drcontext, app_pc pc)
    : drcontext(drcontext)
{
    instr = instr_create(drcontext);
    decode(drcontext, (byte *)pc, instr);
}

DecodedInstruction::DecodedInstruction(const DecodedInstruction &other)
{
    this->drcontext = other.drcontext;
    this->instr = instr_clone(other.drcontext, other.instr);
}

DecodedInstruction::DecodedInstruction(DecodedInstruction &&other)
{
    this->drcontext = other.drcontext;
    this->instr = other.instr;

    other.drcontext = nullptr;
    other.instr = nullptr;
}

DecodedInstruction::~DecodedInstruction()
{
    this->destroy();
}

void DecodedInstruction::destroy()
{
    if (instr != nullptr)
    {
        instr_destroy(drcontext, instr);
        instr = nullptr;
    }
}

disabled_autopredication::disabled_autopredication(instrlist_t *ilist)
    : ilist(ilist)
{
    pred = instrlist_get_auto_predicate(ilist);
    instrlist_set_auto_predicate(ilist, DR_PRED_NONE);
}

disabled_autopredication::~disabled_autopredication()
{
    this->restore();
}

void disabled_autopredication::restore()
{
    if (pred != DR_PRED_NONE)
    {
        instrlist_set_auto_predicate(ilist, pred);
        pred = DR_PRED_NONE;
    }
}

disabled_autopredication::disabled_autopredication(disabled_autopredication &&other)
{
    this->pred = other.pred;
    this->ilist = other.ilist;
    other.pred = DR_PRED_NONE;
}

bool ldr_is_offs_addr(uint raw_instr_bits)
{
    return IS_BIT_UP(raw_instr_bits, 24) &&
           IS_BIT_DOWN(raw_instr_bits, 21);
}

bool ldr_is_pre_addr(uint raw_instr_bits)
{
    return IS_BIT_UP(raw_instr_bits, 24) &&
           IS_BIT_UP(raw_instr_bits, 21);
}

bool ldr_is_pre_or_offs_addr(uint raw_instr_bits)
{
    return IS_BIT_UP(raw_instr_bits, 24);
}

bool ldr_is_post_addr(uint raw_instr_bits)
{
    return IS_BIT_DOWN(raw_instr_bits, 24);
}

void unimplemented_opcode(instr_t *where)
{
    /* N/A */
}

void instrlist_meta_preinsert_xl8(instrlist_t *ilist, instr_t *where, instr_t *insert)
{
    instrlist_meta_preinsert(ilist, where, INSTR_XL8(insert, instr_get_app_pc(where)));
}

void what_are_srcs(instr_t *where)
{
    int n = instr_num_srcs(where);

    if (n == 0)
        dr_printf("No args\n");
    else
    {
        dr_printf("%d args:", n);
        for (int i = 0; i < n; i++)
        {
            opnd_t opnd = instr_get_src(where, i);
            const char *s = opnd_is_reg(opnd)
                                ? "reg"
                                : opnd_is_null(opnd)
                                      ? "null"
                                      : opnd_is_immed(opnd)
                                            ? "imm"
                                            : opnd_is_memory_reference(opnd)
                                                  ? "mem"
                                                  : "unknown";

            dr_printf("%s ", s);
        }

        dr_printf("\n");
    }
}

void what_are_dsts(instr_t *where)
{
    int n = instr_num_dsts(where);

    if (n == 0)
        dr_printf("No args\n");
    else
    {
        dr_printf("%d args:", n);
        for (int i = 0; i < n; i++)
        {
            opnd_t opnd = instr_get_dst(where, i);
            const char *s = opnd_is_reg(opnd)
                                ? "reg"
                                : opnd_is_null(opnd)
                                      ? "null"
                                      : opnd_is_immed(opnd)
                                            ? "imm"
                                            : opnd_is_memory_reference(opnd)
                                                  ? "mem"
                                                  : "unknown";

            dr_printf("%s ", s);
        }

        dr_printf("\n");
    }
}