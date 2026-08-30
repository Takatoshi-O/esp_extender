#include <stdlib.h>
#include <stdio.h>
#include <include/kernel.h>
#include <t_syslog.h>
#include <string.h>

#include "lump_color.h"

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

static uint8_t instanceID = 0;
static lump_color_id_t calib_color;

typedef enum {
    COLOR_SYS_MODE_SYSTEM      = 0,
    COLOR_SYS_MODE_NOT_CALIB   = 1,
    COLOR_SYS_MODE_COLOR_CALIB = 2,
} color_sys_mode_t;

typedef enum
{
    COLOR_SELECT_INSTANCEID,
    COLOR_SELECT_COLOR,
    COLOR_CALIB_MODE_MAX,
} color_calib_mode_t;

static void display_color_calib_mode(color_calib_mode_t mode)
{
    switch (mode)
    {
    case COLOR_SELECT_INSTANCEID:
        hub_display_char('I');
        dly_tsk(500*1000);
        hub_display_number(instanceID);
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


void color_sensor_color_calib(intptr_t exinf)
{
    lump_button_status_t status[LUMP_BTN_MAX];
    lump_button_edge_state_t bt_state = {0};
    color_calib_mode_t color_calib_mode = COLOR_SELECT_INSTANCEID;
    calib_color = LUMP_COLOR_BLACK;

    send_command(esp32_dev, SENSOR_COLOR, COLOR_MODE_SYSTEM, 
                 instanceID, COLOR_SYS_MODE_COLOR_CALIB, 
                 calib_color, 0, 0);
    hub_display_char('I');
    dly_tsk(500*1000);
    hub_display_number(instanceID);

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
            send_command(esp32_dev, SENSOR_COLOR, COLOR_MODE_SYSTEM, 
                         instanceID, COLOR_SYS_MODE_COLOR_CALIB, 
                         calib_color, 1, 0);
            lump_button_wait_release(LUMP_BTN_CENTER);

            continue;
        }

        if (status[LUMP_BTN_RIGHT].pressed)
        {
            switch (color_calib_mode)
            {
            case COLOR_SELECT_INSTANCEID:
                if (instanceID + 1 < LUMP_MAX_INSTANCES_PER_TYPE) instanceID ++;
                else instanceID = 0;
                hub_display_number(instanceID);
                break;
            case COLOR_SELECT_COLOR:
                if (calib_color < LUMP_COLOR_NAVY) calib_color ++;
                else calib_color = LUMP_COLOR_BLACK;
                hub_display_char(lump_color_id_to_char(calib_color));
                break;
            default:
                break;
            }
            lump_button_wait_release(LUMP_BTN_RIGHT);
        }
        
        if (status[LUMP_BTN_LEFT].pressed)
        {
            switch (color_calib_mode)
            {
            case COLOR_SELECT_INSTANCEID:
                if (instanceID > 0) instanceID --;
                else instanceID = LUMP_MAX_INSTANCES_PER_TYPE - 1;
                hub_display_number(instanceID);
                break;
            case COLOR_SELECT_COLOR:
                if (calib_color > LUMP_COLOR_BLACK) calib_color --;
                else calib_color = LUMP_COLOR_NAVY;
                hub_display_char(lump_color_id_to_char(calib_color));
                break;
            default:
                break;
            }
            lump_button_wait_release(LUMP_BTN_LEFT);
        }
        
        if (is_pressed)
        {
            send_command(esp32_dev, SENSOR_COLOR, COLOR_MODE_SYSTEM, 
                         instanceID, COLOR_SYS_MODE_COLOR_CALIB, 
                         calib_color, 0, 0);
        }
        
        dly_tsk(50*1000);
    }
}