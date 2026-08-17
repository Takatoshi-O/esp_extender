#include <stdlib.h>
#include <stdio.h>
#include <include/kernel.h>
#include "kernel_cfg.h"

#include "lump_comm.h"
#include "lump_comm_tsk.h"
#include "header/lump_receive.h"
#include "header/lump_dispatch.h"

static lump_command_handler_t s_handlers[SENSOR_TYPE_MAX] = {0};

void lump_command_dispatch_register(lump_sensor_type_t type, lump_command_handler_t handler) {
    if (type >= SENSOR_TYPE_MAX) return;
    s_handlers[type] = handler;
}

void lump_dispatch_poll(intptr_t exinf) {
    lump_command_entry_t entry;
    while (true) 
    {
        if (lump_command_pop(&entry))
        {
            if (entry.type >= SENSOR_TYPE_MAX) continue;
            lump_command_handler_t handler = s_handlers[entry.type];
            if (handler != NULL) {
                handler(entry);
            }
        }
        dly_tsk(5000);
    }
}
