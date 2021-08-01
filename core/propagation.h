#include "dr_api.h"

void
propagate(void *drcontext, instrlist_t *ilist, instr_t *where);

struct PropagationResult
{
	bool errno;
};