#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "lump_sensors.h"

/*
 * カラーセンサーのモード定義。
 * 0: 初期設定用
 * 1: RGB値
 * 2: 色ID
 */
typedef enum {
    COLOR_MODE_SYSTEM = 0,
    COLOR_MODE_RGBC = 1,
    COLOR_MODE_COLOR_ID = 2,
    COLOR_MODE_NOTIFY_COLOR = 3,
} lump_color_sensor_mode_t;

typedef struct 
{
    int16_t r, g, b, c;
} lump_color_rgbc_t;

void lump_color_init();

void lump_color_setup(uint8_t instanceID);

lump_color_id_t lump_color_get_color(uint8_t instanceID);

lump_color_rgbc_t lump_color_get_rgbc(uint8_t instanceID);

lump_color_id_t lump_color_get_notify_color(uint8_t instanceID);

void lump_color_request_calib(uint8_t instanceID, lump_color_id_t colorID);

void lump_color_set_notify_color(uint8_t instanceID, lump_color_id_t colorID, bool isadd);