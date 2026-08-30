#pragma once

#include <stdlib.h>
#include <stdbool.h>

#include "lump_sensors.h"

typedef enum {
    CAMERA_MODE_SYSTEM      = 0,
    CAMERA_MODE_POS_COLOR   = 1,
    CAMERA_MODE_12POS_COLOR = 2,
} camera_mode_t;

typedef struct 
{
    int16_t y, u, v;
} lump_camera_yuv_t;

void lump_camera_sta_calib();

void lump_camera_init();

void lump_camera_setup(uint8_t instanceID);

lump_color_id_t lump_camera_get_color(uint8_t instanceID, int16_t x, int16_t y, uint8_t radius);

lump_camera_yuv_t lump_camera_get_yuv(uint8_t instanceID, int16_t x, int16_t y);

bool lump_camera_get_12pos_color(lump_color_id_t color_ids[12], uint8_t instanceID, 
                                    uint8_t use_datalist, uint8_t radius);