#pragma once

#include <stdlib.h>
#include "lump_comm.h"

#define LUMP_COMMAND_QUEUE_CAPACITY 16

typedef struct {
    lump_sensor_type_t type;
    uint8_t command;      /* byte0下位5bit */
    uint8_t seq;
    uint8_t instance_id;
    int16_t v1, v2, v3, v4;
} lump_command_entry_t;

bool lump_command_pop(lump_command_entry_t *out);

void lump_command_push(const uint8_t raw[LUMP_PAYLOAD_LEN]);