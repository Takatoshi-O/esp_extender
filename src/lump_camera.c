#include <stdlib.h>
#include <stdio.h>
#include <include/kernel.h>
#include <t_syslog.h>
#include <string.h>

#include "lump_camera.h"

#include "lump_comm.h"
#include "header/lump_dispatch.h"
#include "lump_sensors.h"

#include "kernel_cfg.h"
#include "syssvc/serial.h"

typedef struct {
    int16_t color_id;
    int16_t y, u, v;
    bool isnew;
} camera_sensor_value_t;
static camera_sensor_value_t camera_sensor_values[LUMP_MAX_INSTANCES_PER_TYPE];

typedef struct 
{
    lump_color_id_t color_ids[12];
    uint8_t datalist;
    bool isnew;
} camera_12pos_value_t;
static camera_12pos_value_t camera_12pos_values[LUMP_MAX_INSTANCES_PER_TYPE]; 

void lump_camera_sta_calib()
{
    act_tsk(LUMP_CHANGE_CAM_CALIB_MODE_TSK);
    act_tsk(LUMP_CAMERA_CALIB_TSK);
}

static void lump_camera(lump_command_entry_t data);

void lump_camera_init()
{
    memset(camera_sensor_values, 0, sizeof(camera_sensor_values));
    memset(camera_12pos_values, 0, sizeof(camera_12pos_values));
    lump_command_dispatch_register(SENSOR_CAMERA, lump_camera);
}


static void unpack_12pos_color(int16_t raw_data[3], uint8_t color_ids[12])
{
    for (int i = 0; i < 3; i++)
    {
        color_ids[0+4*i] = (uint8_t)((raw_data[i] >> 12) & 0x0F);
        color_ids[1+4*i] = (uint8_t)((raw_data[i] >> 8) & 0x0F);
        color_ids[2+4*i] = (uint8_t)((raw_data[i] >> 4) & 0x0F);
        color_ids[3+4*i] = (uint8_t)(raw_data[i] & 0x0F);
    }
}


static void lump_camera(lump_command_entry_t data)
{
    switch (data.command)
    {
    case CAMERA_MODE_SYSTEM:
        break;
    case CAMERA_MODE_POS_COLOR:
        camera_sensor_values[data.instance_id].y = data.v1;
        camera_sensor_values[data.instance_id].u = data.v2;
        camera_sensor_values[data.instance_id].v = data.v3;
        camera_sensor_values[data.instance_id].color_id = data.v4;
        camera_sensor_values[data.instance_id].isnew = true;
        break;
    case CAMERA_MODE_12POS_COLOR:
        camera_12pos_values[data.instance_id].datalist = data.v1;
        int16_t raw_data[3];
        raw_data[0] = data.v2; raw_data[1] = data.v3; raw_data[1] = data.v4;
        unpack_12pos_color(raw_data, (uint8_t *)camera_12pos_values[data.instance_id].color_ids);
        camera_12pos_values[data.instance_id].isnew = true;
        break;
    default:
        break;
    }
}

lump_color_id_t lump_camera_color(uint8_t instanceID, int16_t x, int16_t y, uint8_t px_size)
{
    lump_color_id_t colorID;
    send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_POS_COLOR, instanceID, x, y, (int16_t)px_size, 0);
    for (int i = 0; i < 200; i++)
    {
        if (camera_sensor_values[instanceID].isnew)
        {
            camera_sensor_values[instanceID].isnew = false;
            colorID = camera_sensor_values[instanceID].color_id;
            return colorID;
        }
        dly_tsk(10000);
    }
    colorID = LUMP_COLOR_ERROR;
    return colorID;
}

lump_camera_yuv_t lump_camera_get_yuv(uint8_t instanceID, int16_t x, int16_t y)
{
    lump_camera_yuv_t yuv;
    send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_POS_COLOR, instanceID, x, y, 1, 0);
    for (int i = 0; i < 200; i++)
    {
        if (camera_sensor_values[instanceID].isnew)
        {
            camera_sensor_values[instanceID].isnew = false;
            yuv.y = camera_sensor_values[instanceID].y;
            yuv.u = camera_sensor_values[instanceID].u;
            yuv.v = camera_sensor_values[instanceID].v;
            return yuv;
        }
        dly_tsk(10000);
    }

    yuv.y = -1; yuv.u = -1; yuv.v = -1;
    return yuv;
}


bool lump_camera_get_12pos_color(lump_color_id_t color_ids[12], uint8_t instanceID, uint8_t use_datalist, uint8_t px_size)
{
    send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_12POS_COLOR, instanceID, (int16_t)use_datalist, (int16_t)px_size, 0, 0);
    for (int i = 0; i < 200; i++)
    {
        if (camera_12pos_values[instanceID].isnew)
        {
            for (int i = 0; i < 12; i++)
            {
                color_ids[i] = camera_12pos_values[instanceID].color_ids[i];
            }
            return true;
        }
        dly_tsk(10000);
    }
    
    return false;
}