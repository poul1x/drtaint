#pragma once

#include "dr_api.h"
#include "defines.h"

#ifdef __cplusplus
extern "C" {
#endif

bool
drtaint_init(client_id_t id);

void
drtaint_exit(void);

#ifdef __cplusplus
}
#endif