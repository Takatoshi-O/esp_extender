#include <stdlib.h>
#include <stdio.h>
#include <include/kernel.h>
#include <t_syslog.h>
#include <string.h>

#include "lump_camera.h"

#include "spike/hub/button.h"
#include "spike/hub/display.h"
#include "pbio/control.h"

#include "lump_comm.h"
#include "lump_comm_tsk.h"
#include "lump_sensors.h"

#include "kernel_cfg.h"
#include "syssvc/serial.h"

#define POS_NUM_MAX 12


static hub_button_t pressed;
static uint8_t pos_list = 0;
static uint8_t pos;
static bool ischange_calib_mode = false;
static uint8_t instanceID = 0;
static lump_color_id_t calib_color;

typedef enum
{
    NOT_CALIB,
    COLOR_CALIB,
    POS_CALIB,
    CALIB_MODE_MAX,
} calib_mode_t;
static calib_mode_t calib_mode;

typedef enum
{
    POS_SELECT_POSLIST,
    POS_SELECT_POSNUM,
    POS_MOVE_POS,
    POS_CALIB_MODE_MAX,
} pos_calib_mode_t;
static pos_calib_mode_t pos_calib_mode;

typedef enum
{
    COLOR_SELECT_POSLIST,
    COLOR_SELECT_POSNUM,
    COLOR_SELECT_COLOR,
    COLOR_CALIB_MODE_MAX,
} color_calib_mode_t;
static color_calib_mode_t color_calib_mode;

typedef enum
{
    HORIZONTAL = 1,
    VERTICAL = -1,
} direction_t;
direction_t direction;

static uint16_t pack_list_and_pos(uint8_t list, uint8_t pos)
{
    uint16_t out;

    out = ((uint16_t)list<< 8) | ((uint16_t)pos);
    return out;
}

static void display_calib_mode(calib_mode_t mode)
{
    switch (mode)
    {
    case NOT_CALIB:
        hub_display_char('N');
        break;
    case COLOR_CALIB:
        hub_display_char('C');
        break;
    case POS_CALIB:
        hub_display_char('P');
        break;
    default:
        break;
    }
}

static void display_pos_calib_mode(pos_calib_mode_t mode)
{
    switch (mode)
    {
    case POS_SELECT_POSLIST:
        hub_display_char('L');
        dly_tsk(500*1000);
        hub_display_number(pos_list);
        break;
    case POS_SELECT_POSNUM:
        hub_display_char('P');
        dly_tsk(500*1000);
        hub_display_number(pos);
        break;
    case POS_MOVE_POS:
        hub_display_char('M');
        dly_tsk(500*1000);
        if(direction == HORIZONTAL) hub_display_char('H');
        else hub_display_char('V');
        break;
    default:
        break;
    }
}

static void display_color_calib_mode(color_calib_mode_t mode)
{
    switch (mode)
    {
    case COLOR_SELECT_POSLIST:
        hub_display_char('L');
        dly_tsk(500*1000);
        hub_display_number(pos_list);
        break;
    case COLOR_SELECT_POSNUM:
        hub_display_char('P');
        dly_tsk(500*1000);
        hub_display_number(pos);
        break;
    case COLOR_SELECT_COLOR:
        hub_display_char('C');
        dly_tsk(500*1000);
        hub_display_char(lump_color_id_to_char(calib_color));
        break;
    default:
        break;
    }
}

static void change_calib_mode()
{
    display_calib_mode(calib_mode);
    while (true)
    {
        while (true)
        {
            hub_button_is_pressed(&pressed);
            if(pressed != 0) break;
            dly_tsk(50*1000);
        }
        if (pressed & HUB_BUTTON_CENTER) break;
        if (pressed & HUB_BUTTON_RIGHT)
        {
            calib_mode = (calib_mode + 1) % CALIB_MODE_MAX;
            display_calib_mode(calib_mode);
        }
        if (pressed & HUB_BUTTON_LEFT)
        {
            if (calib_mode > 0) calib_mode = (calib_mode - 1) % CALIB_MODE_MAX;
            else calib_mode = CALIB_MODE_MAX - 1;
            display_calib_mode(calib_mode);
        }
        dly_tsk(200*1000);
    }
    if (calib_mode == COLOR_CALIB || calib_mode == POS_CALIB)
    {
        hub_display_char('I');
        dly_tsk(500*1000);
        hub_display_number(instanceID);
        while (true)
        {
            while (true)
            {
                hub_button_is_pressed(&pressed);
                if(pressed != 0) break;
                dly_tsk(50*1000);
            }
            if (pressed & HUB_BUTTON_CENTER) break;
            if (pressed & HUB_BUTTON_RIGHT)
            {
                instanceID = (instanceID + 1) % 255;
                hub_display_number(instanceID);
            }
            if (pressed & HUB_BUTTON_LEFT)
            {
                if (instanceID > 0) instanceID = (instanceID-1) % 255;
                hub_display_number(instanceID);
            }
            dly_tsk(200*1000);
        }
        pos_calib_mode = POS_SELECT_POSNUM;
        color_calib_mode = COLOR_SELECT_POSNUM;
        calib_color = LUMP_COLOR_BLACK;
        direction = HORIZONTAL;
        pos = 0;

        if (calib_mode == COLOR_CALIB) 
        {
            display_color_calib_mode(color_calib_mode);
            pos_list = 0;
        }
        if (calib_mode == POS_CALIB) 
        {
            display_pos_calib_mode(pos_calib_mode);
            pos_list = 1;
        }
    }
}

void lump_change_cam_calib_mode(intptr_t exinf)
{
    hub_button_t BTpressed;
    hub_button_is_pressed(&BTpressed);

    while (true)
    {
        while (!(BTpressed & HUB_BUTTON_BT))
        {
            dly_tsk(50*1000);
            hub_button_is_pressed(&BTpressed);
        }
        ischange_calib_mode = true;
        uint32_t pressed_start = pbio_control_get_time_ticks();
        while (pbio_control_get_time_ticks() - pressed_start < pbio_control_time_ms_to_ticks(1000))
        {
            hub_button_is_pressed(&BTpressed);
            if (!(BTpressed & HUB_BUTTON_BT)) break;
            dly_tsk(50*1000);
        }

        if (BTpressed & HUB_BUTTON_BT) 
        {
            change_calib_mode();
            dly_tsk(300 * 1000);
            if (calib_mode == POS_CALIB || calib_mode == COLOR_CALIB) hub_display_number(pos);
            else hub_display_off();
        }
        else
        {
            if (calib_mode == POS_CALIB)
            {
                pos_calib_mode = (pos_calib_mode + 1) % POS_CALIB_MODE_MAX;
                display_pos_calib_mode(pos_calib_mode);
            }
            if (calib_mode == COLOR_CALIB)
            {
                color_calib_mode = (color_calib_mode + 1) % COLOR_CALIB_MODE_MAX;
                display_color_calib_mode(color_calib_mode);
            }
        }

        ischange_calib_mode = false;
        while (BTpressed & HUB_BUTTON_BT)
        {
            dly_tsk(50*1000);
            hub_button_is_pressed(&BTpressed);
        }
    }
}

static void color_calib()
{
    if (pressed & HUB_BUTTON_RIGHT)
    {
        switch (color_calib_mode)
        {
        case COLOR_SELECT_POSLIST:
            if (pos_list < 255) pos_list++;
            hub_display_number(pos_list);
            break;
        case COLOR_SELECT_COLOR:
            calib_color = (calib_color + 1) % 8;
            hub_display_char(lump_color_id_to_char(calib_color));
            break;
        case COLOR_SELECT_POSNUM:
            pos = (pos+1) % POS_NUM_MAX;
            hub_display_number(pos);
        default:
            break;
        }
        send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, instanceID, LUMP_CAM_CALIB_COLOR, (int16_t)pack_list_and_pos(pos_list, pos), calib_color, 0);
        dly_tsk(200*1000);
    }
    if (pressed & HUB_BUTTON_LEFT)
    {
        switch (color_calib_mode)
        {
        case COLOR_SELECT_POSLIST:
            if (pos_list > 0) pos_list--;
            hub_display_number(pos_list);
            break;
        case COLOR_SELECT_COLOR:
            if (calib_color > 0) calib_color = (calib_color - 1) % 8;
            else calib_color = LUMP_COLOR_PURPLE;
            hub_display_char(lump_color_id_to_char(calib_color));
            break;
        case COLOR_SELECT_POSNUM:
            if (pos > 0) pos = (pos-1) % POS_NUM_MAX;
            else pos = POS_NUM_MAX - 1;
            hub_display_number(pos);
            break;
        default:
            break;
        }
        send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, instanceID, LUMP_CAM_CALIB_COLOR, (int16_t)pack_list_and_pos(pos_list, pos), calib_color, 0);
        dly_tsk(200*1000);
    }
    if (pressed & HUB_BUTTON_CENTER)
    {
        send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, instanceID, LUMP_CAM_CALIB_COLOR, (int16_t)pack_list_and_pos(pos_list, pos), calib_color, 1);
        while (pressed & HUB_BUTTON_CENTER)
        {
            hub_button_is_pressed(&pressed);
            dly_tsk(50 * 1000);
        }
    }
}

static void pos_calib()
{
    uint32_t pressed_start = pbio_control_get_time_ticks();
    if (pressed & HUB_BUTTON_RIGHT)
    {
        switch (pos_calib_mode)
        {
        case POS_SELECT_POSLIST:
            if (pos_list < 255) pos_list++;
            hub_display_number(pos_list);
            send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, instanceID, LUMP_CAM_CALIB_POS, 
                        (int16_t)pack_list_and_pos(pos_list, pos), 0, 0);
            dly_tsk(200*1000);
            break;
        case POS_SELECT_POSNUM:
            pos = (pos+1) % POS_NUM_MAX;
            hub_display_number(pos);
            send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, instanceID, LUMP_CAM_CALIB_POS, 
                        (int16_t)pack_list_and_pos(pos_list, pos), 0, 0);
            dly_tsk(200*1000);
            break;
        case POS_MOVE_POS:
            while (pressed & HUB_BUTTON_RIGHT)
            {
                if ((pbio_control_get_time_ticks() - pressed_start) < pbio_control_time_ms_to_ticks(1200))
                {
                    send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, instanceID, LUMP_CAM_CALIB_POS, 
                        (int16_t)pack_list_and_pos(pos_list, pos), 1, direction);
                }
                else
                {
                    send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, instanceID, LUMP_CAM_CALIB_POS, 
                        (int16_t)pack_list_and_pos(pos_list, pos), 5, direction);
                }
                dly_tsk(200*1000);
                hub_button_is_pressed(&pressed);
            }
            break;
        default:
            break;
        }
    }
    if (pressed & HUB_BUTTON_LEFT)
    {
        switch (pos_calib_mode)
        {
        case POS_SELECT_POSLIST:
            if (pos_list > 1) pos_list--;
            hub_display_number(pos_list);
            send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, instanceID, LUMP_CAM_CALIB_POS, 
                        (int16_t)pack_list_and_pos(pos_list, pos), 0, 0);
            dly_tsk(200*1000);
            break;
        case POS_SELECT_POSNUM:
            if (pos > 0) pos = (pos-1) % POS_NUM_MAX;
            else pos = POS_NUM_MAX - 1;
            hub_display_number(pos);
            send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, instanceID, LUMP_CAM_CALIB_POS, 
                        (int16_t)pack_list_and_pos(pos_list, pos), 0, 0);
            dly_tsk(200*1000);
            break;
        case POS_MOVE_POS:
            while (pressed & HUB_BUTTON_LEFT)
            {
                if ((pbio_control_get_time_ticks() - pressed_start) < pbio_control_time_ms_to_ticks(1200))
                {
                    send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, instanceID, LUMP_CAM_CALIB_POS, 
                        (int16_t)pack_list_and_pos(pos_list, pos), -1, direction);
                }
                else
                {
                    send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, instanceID, LUMP_CAM_CALIB_POS, 
                        (int16_t)pack_list_and_pos(pos_list, pos), -5, direction);
                }
                dly_tsk(200*1000);
                hub_button_is_pressed(&pressed);
            }
            break;
        default:
            break;
        }
    }
    if (pressed & HUB_BUTTON_CENTER)
    {
        if (pos_calib_mode == POS_MOVE_POS)
        {
            direction *= -1;
            if (direction == HORIZONTAL) hub_display_char('H');
            else hub_display_char('V');
            dly_tsk(300*1000);
            while (pressed & HUB_BUTTON_CENTER)
            {
                hub_button_is_pressed(&pressed);
                dly_tsk(50 * 1000);
            }
        }
    }
}


void lump_camera_calib(intptr_t exinf)
{
    while (true)
    {
        if (!ischange_calib_mode)
        {
            hub_button_is_pressed(&pressed);
            if (pressed != 0)
            {
                switch (calib_mode)
                {
                case POS_CALIB:
                    pos_calib();
                    break;
                case COLOR_CALIB:
                    color_calib();
                    break;
                default:
                    break;
                }
            }
        }
        dly_tsk(50*1000);
    }
}

