#include <stdlib.h>
#include <stdio.h>
#include <include/kernel.h>
#include <t_syslog.h>
#include <string.h>

#include "lump_color.h"

#include "lump_comm.h"
#include "lump_comm_tsk.h"
#include "header/lump_receive.h"
#include "lump_sensors.h"
#include "header/lump_dispatch.h"

/* 監視色として指定できる色IDの範囲(ビットマスクで管理するための上限) */
#define COLOR_SENSOR_MAX_WATCH_COLOR_ID 16

typedef struct {
    int16_t color_id;
    int16_t r, g, b, c;
    bool isnew;
} color_sensor_value_t;

static color_sensor_value_t color_sensor_values[LUMP_MAX_INSTANCES_PER_TYPE];
static int16_t color_sensor_notify_values[LUMP_MAX_INSTANCES_PER_TYPE];

static void lump_color(lump_command_entry_t data);

void lump_color_init()
{
    memset(color_sensor_values, 0, sizeof(color_sensor_values));
    lump_command_dispatch_register(SENSOR_COLOR, lump_color);
}

void lump_color_setup(uint8_t instanceID)
{
    send_command(esp32_dev, SENSOR_COLOR, COLOR_MODE_SYSTEM, instanceID, 0, 1, 0, 0);
}

static void lump_color(lump_command_entry_t data)
{
    switch (data.command)
    {
    case COLOR_MODE_SYSTEM:
        break;
    case COLOR_MODE_RGBC:
        color_sensor_values[data.instance_id].r = data.v1;
        color_sensor_values[data.instance_id].g = data.v2;
        color_sensor_values[data.instance_id].b = data.v3;
        color_sensor_values[data.instance_id].c = data.v4;
        color_sensor_values[data.instance_id].isnew = true;
        break;
    case COLOR_MODE_COLOR_ID:
        color_sensor_values[data.instance_id].color_id = data.v1;
        color_sensor_values[data.instance_id].isnew = true;
        break;
    case COLOR_MODE_NOTIFY_COLOR:
        color_sensor_notify_values[data.instance_id] = data.v1;
        break;
    default:
        break;
    }
}

lump_color_id_t lump_color_get_color(uint8_t instanceID)
{
    lump_color_id_t colorID;
    send_command(esp32_dev, SENSOR_COLOR, COLOR_MODE_COLOR_ID, instanceID, 0, 0, 0, 0);
    for (int i = 0; i < 50; i++)
    {
        if (color_sensor_values[instanceID].isnew)
        {
            color_sensor_values[instanceID].isnew = false;
            colorID = color_sensor_values[instanceID].color_id;
            return colorID;
        }
        dly_tsk(5000);
    }
    
    colorID = LUMP_COLOR_ERROR;
    return colorID;
}

lump_color_rgbc_t lump_color_get_rgbc(uint8_t instanceID)
{
    lump_color_rgbc_t rgbc;
    send_command(esp32_dev, SENSOR_COLOR, COLOR_MODE_RGBC, instanceID, 0, 0, 0, 0);
    for (int i = 0; i < 50; i++)
    {
        if (color_sensor_values[instanceID].isnew)
        {
            color_sensor_values[instanceID].isnew = false;
            rgbc.r = color_sensor_values[instanceID].r;
            rgbc.g = color_sensor_values[instanceID].g;
            rgbc.b = color_sensor_values[instanceID].b;
            rgbc.c = color_sensor_values[instanceID].c;
            return rgbc;
        }
        dly_tsk(5000);
    }

    rgbc.r = -1; rgbc.g = -1; rgbc.b = -1; rgbc.c = -1;
    return rgbc;
}

lump_color_id_t lump_color_get_notify_color(uint8_t instanceID)
{
    return (lump_color_id_t)color_sensor_notify_values[instanceID];
}

void lump_color_request_calib(uint8_t instanceID, lump_color_id_t colorID)
{
    send_command(esp32_dev, SENSOR_COLOR, COLOR_MODE_SYSTEM, instanceID, 1, colorID, 0, 0);
}


void lump_color_set_notify_color(uint8_t instanceID, lump_color_id_t colorID, bool isadd)
{
    send_command(esp32_dev, SENSOR_COLOR, COLOR_MODE_NOTIFY_COLOR, instanceID, (int16_t)colorID, (int16_t)isadd, 0, 0);
}