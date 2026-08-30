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

#include "lump_calib.h"
#include "header/lump_button_input.h"

#include "kernel_cfg.h"
#include "syssvc/serial.h"

typedef enum {
    CAMERA_SYS_MODE_SYSTEM      = 0,
    CAMERA_SYS_MODE_NOT_CALIB   = 1,
    CAMERA_SYS_MODE_COLOR_CALIB = 2,
    CAMERA_SYS_MODE_POS_CALIB   = 3,
} camera_sys_mode_t;

#define POS_NUM_MAX 12

static uint8_t instanceID = 0;

typedef enum
{
    POS_SELECT_POSLIST,
    POS_SELECT_POSNUM,
    POS_MOVE_POS,
    POS_CALIB_MODE_MAX,
} pos_calib_mode_t;

typedef enum
{
    COLOR_SELECT_POSLIST,
    COLOR_SELECT_POSNUM,
    COLOR_SELECT_COLOR,
    COLOR_CALIB_MODE_MAX,
} color_calib_mode_t;

typedef enum
{
    HORIZONTAL = 1,
    VERTICAL = -1,
} direction_t;

static uint8_t pos;
static uint8_t pos_list;
static direction_t direction;
static lump_color_id_t calib_color;


static uint16_t pack_list_and_pos(uint8_t list, uint8_t pos)
{
    uint16_t out;

    out = ((uint16_t)list<< 8) | ((uint16_t)pos);
    return out;
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


void cam_color_calib(intptr_t exinf)
{
    lump_button_status_t status[LUMP_BTN_MAX];
    lump_button_edge_state_t bt_state = {0};
    color_calib_mode_t color_calib_mode = COLOR_SELECT_POSNUM;
    calib_color = LUMP_COLOR_BLACK;
    pos = 0;
    pos_list = 0;

    send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, 
                 instanceID, CAMERA_SYS_MODE_COLOR_CALIB, 
                 (int16_t)pack_list_and_pos(pos_list, pos), calib_color, 0);
    hub_display_char('I');
    dly_tsk(500*1000);
    hub_display_number(instanceID);
    
    while (true)
    {
        lump_button_query_status(status);
        if (status[LUMP_BTN_CENTER].pressed) break;

        if (status[LUMP_BTN_RIGHT].pressed) 
        {
            if (instanceID + 1 < LUMP_MAX_INSTANCES_PER_TYPE) instanceID ++;
            else instanceID = 0;
            hub_display_number(instanceID);
            lump_button_wait_release(LUMP_BTN_RIGHT);
        }

        if (status[LUMP_BTN_LEFT].pressed)
        {
            if (instanceID > 0) instanceID --;
            else instanceID = LUMP_MAX_INSTANCES_PER_TYPE - 1;
            hub_display_number(instanceID);
            lump_button_wait_release(LUMP_BTN_LEFT);
        }

        dly_tsk(50*1000);
    }
    
    send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, 
                 instanceID, CAMERA_SYS_MODE_COLOR_CALIB, 
                 (int16_t)pack_list_and_pos(pos_list, pos), calib_color, 0);
    display_color_calib_mode(color_calib_mode);

    while (!is_change_calib_mode())
    {
        bool is_pressed = lump_button_query_status(status);

        lump_button_event_t bt_event;
        if (lump_button_detect_edge(status, LUMP_BTN_BT, &bt_state, &bt_event) &&
            bt_event.kind == LUMP_PRESS_SHORT)
        {
            color_calib_mode = (color_calib_mode + 1) % COLOR_CALIB_MODE_MAX;
            display_color_calib_mode(color_calib_mode);
        }

        if (status[LUMP_BTN_CENTER].pressed)
        {
            send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, 
                         instanceID, CAMERA_SYS_MODE_COLOR_CALIB, 
                         (int16_t)pack_list_and_pos(pos_list, pos), calib_color, 1);
            lump_button_wait_release(LUMP_BTN_CENTER);

            continue;
        }

        if (status[LUMP_BTN_RIGHT].pressed)
        {
            switch (color_calib_mode)
            {
            case COLOR_SELECT_POSLIST:
                if (pos_list < 255) pos_list++;
                hub_display_number(pos_list);
                break;
            case COLOR_SELECT_COLOR:
                if (calib_color < 7) calib_color ++;
                else calib_color = LUMP_COLOR_BLACK;
                hub_display_char(lump_color_id_to_char(calib_color));
                break;
            case COLOR_SELECT_POSNUM:
                pos = (pos+1) % POS_NUM_MAX;
                hub_display_number(pos);
            default:
                break;
            }
            lump_button_wait_release(LUMP_BTN_RIGHT);
        }
        
        if (status[LUMP_BTN_LEFT].pressed)
        {
            switch (color_calib_mode)
            {
            case COLOR_SELECT_POSLIST:
                if (pos_list > 0) pos_list--;
                hub_display_number(pos_list);
                break;
            case COLOR_SELECT_COLOR:
                if (calib_color > 1) calib_color --;
                else calib_color = LUMP_COLOR_ORANGE;

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
            lump_button_wait_release(LUMP_BTN_LEFT);
        }
        
        if (is_pressed)
        {
            send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, 
                         instanceID, CAMERA_SYS_MODE_COLOR_CALIB, 
                         (int16_t)pack_list_and_pos(pos_list, pos), calib_color, 0);
        }
        
        dly_tsk(50*1000);
    }
}


void cam_pos_calib(intptr_t exinf)
{
    lump_button_status_t status[LUMP_BTN_MAX];
    lump_button_edge_state_t bt_state = {0};
    pos_calib_mode_t pos_calib_mode = POS_SELECT_POSNUM;
    pos = 0;
    pos_list = 1;
    direction = HORIZONTAL;
    int move_range = 0;

    send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, 
                 instanceID, CAMERA_SYS_MODE_POS_CALIB, 
                 (int16_t)pack_list_and_pos(pos_list, pos), 0, direction);
    hub_display_char('I');
    dly_tsk(500*1000);
    hub_display_number(instanceID);
    
    while (true)
    {
        lump_button_query_status(status);
        if (status[LUMP_BTN_CENTER].pressed) break;

        if (status[LUMP_BTN_RIGHT].pressed) 
        {
            if (instanceID + 1 < LUMP_MAX_INSTANCES_PER_TYPE) instanceID ++;
            else instanceID = 0;
            hub_display_number(instanceID);
            lump_button_wait_release(LUMP_BTN_RIGHT);
        }

        if (status[LUMP_BTN_LEFT].pressed)
        {
            if (instanceID > 0) instanceID --;
            else instanceID = LUMP_MAX_INSTANCES_PER_TYPE - 1;
            hub_display_number(instanceID);
            lump_button_wait_release(LUMP_BTN_LEFT);
        }
        dly_tsk(50*1000);
    }
    
    send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, 
                 instanceID, CAMERA_SYS_MODE_POS_CALIB, 
                 (int16_t)pack_list_and_pos(pos_list, pos), 0, direction);
    display_pos_calib_mode(pos_calib_mode);

    while (!is_change_calib_mode())
    {
        bool is_pressed = lump_button_query_status(status);

        lump_button_event_t bt_event;
        if (lump_button_detect_edge(status, LUMP_BTN_BT, &bt_state, &bt_event) &&
            bt_event.kind == LUMP_PRESS_SHORT)
        {
            pos_calib_mode = (pos_calib_mode + 1) % POS_CALIB_MODE_MAX;
            display_pos_calib_mode(pos_calib_mode);
        }

        if (status[LUMP_BTN_CENTER].pressed && pos_calib_mode == POS_MOVE_POS)
        {
            direction *= -1;
            if (direction == HORIZONTAL) hub_display_char('H');
            else hub_display_char('V');

            lump_button_wait_release(LUMP_BTN_CENTER);
            continue;
        }

        move_range = 0;

        if (status[LUMP_BTN_RIGHT].pressed)
        {
            switch (pos_calib_mode)
            {
            case POS_SELECT_POSLIST:
                if (pos_list < 255) pos_list++;
                hub_display_number(pos_list);
                lump_button_wait_release(LUMP_BTN_RIGHT);
                break;
            case POS_SELECT_POSNUM:
                pos = (pos+1) % POS_NUM_MAX;
                hub_display_number(pos);
                lump_button_wait_release(LUMP_BTN_RIGHT);
                break;
            case POS_MOVE_POS:
                if (status[LUMP_BTN_RIGHT].held_ms < 1200) move_range = 1;
                else move_range = 5;
                break;
            default:
                break;
            }
        }
        
        if (status[LUMP_BTN_LEFT].pressed)
        {
            switch (pos_calib_mode)
            {
            case POS_SELECT_POSLIST:
                if (pos_list > 1) pos_list--;
                hub_display_number(pos_list);
                lump_button_wait_release(LUMP_BTN_LEFT);
                break;
            case POS_SELECT_POSNUM:
                if (pos > 0) pos = (pos-1) % POS_NUM_MAX;
                else pos = POS_NUM_MAX - 1;
                hub_display_number(pos);
                lump_button_wait_release(LUMP_BTN_LEFT);
                break;
            case POS_MOVE_POS:
                if (status[LUMP_BTN_LEFT].held_ms < 1200) move_range = -1;
                else move_range = -5;
                break;
            default:
                break;
            }
        }
        
        if (is_pressed)
        {
            send_command(esp32_dev, SENSOR_CAMERA, CAMERA_MODE_SYSTEM, 
                         instanceID, CAMERA_SYS_MODE_POS_CALIB, 
                         (int16_t)pack_list_and_pos(pos_list, pos), move_range, direction);
        }
        
        dly_tsk(50*1000);
    }
}