#include <stdlib.h>
#include <stdio.h>
#include <include/kernel.h>
#include <t_syslog.h>
#include <string.h>

#include "spike/hub/button.h"
#include "spike/hub/display.h"
#include "pbio/control.h"

#include "header/lump_button_input.h"
#include "lump_calib.h"

#include "lump_comm.h"
#include "lump_comm_tsk.h"
#include "lump_sensors.h"

#include "kernel_cfg.h"
#include "syssvc/serial.h"

#define CALIB_MODE 0

typedef enum
{
    NOT_CALIB         = 1,
    CAM_COLOR_CALIB   = 2,
    CAM_POS_CALIB     = 3,
    COLOR_COLOR_CALIB = 4,
    CALIB_MODE_MAX,
} calib_mode_t;

static calib_mode_t calib_mode = NOT_CALIB;
bool is_change_mode = false;

static void display_calib_mode(calib_mode_t mode)
{
    switch (mode)
    {
    case NOT_CALIB:
        hub_display_char('N');
        break;
    case CAM_COLOR_CALIB:
        hub_display_char('C');
        break;
    case CAM_POS_CALIB:
        hub_display_char('P');
        break;
    case COLOR_COLOR_CALIB:
        hub_display_char('C');
    default:
        break;
    }
}

void lump_sta_calib()
{
    act_tsk(LUMP_BUTTON_TSK);
    act_tsk(LUMP_CHANGE_CALIB_MODE_TSK);

}

bool is_change_calib_mode()
{
    return is_change_mode;
}

void change_calib_mode(intptr_t exinf)
{
    lump_button_edge_state_t bt_state = {0};
    lump_button_status_t status[LUMP_BTN_MAX];

    while (true)
    {
        lump_button_query_status(status);

        lump_button_event_t event;
        bool fired = lump_button_detect_edge(status, LUMP_BTN_BT, &bt_state, &event);
        if (!fired || event.kind != LUMP_PRESS_LONG)
        {
            dly_tsk(50*1000);
            continue;
        }

        is_change_mode = true;
        display_calib_mode(calib_mode);
        while (true)
        {
            bool is_pressed = lump_button_query_status(status);
            if (status[LUMP_BTN_CENTER].pressed) break;

            if (status[LUMP_BTN_RIGHT].pressed) 
            {
                if (calib_mode + 1 < CALIB_MODE_MAX) calib_mode ++;
                else calib_mode = NOT_CALIB;
                lump_button_wait_release(LUMP_BTN_RIGHT);
            }

            if (status[LUMP_BTN_LEFT].pressed)
            {
                if (calib_mode > 1) calib_mode = (calib_mode - 1) % CALIB_MODE_MAX;
                else calib_mode = CALIB_MODE_MAX - 1;
                lump_button_wait_release(LUMP_BTN_LEFT);
            }

            if (is_pressed)
            {
                display_calib_mode(calib_mode);
                send_command(esp32_dev, SENSOR_SYSTEM, 0, 0, calib_mode, 0, 0, 0);
            }

            dly_tsk(50*1000);
        }

        is_change_mode = false;

        switch (calib_mode)
        {
        case NOT_CALIB:
            send_command(esp32_dev, SENSOR_CAMERA, CALIB_MODE, 0, NOT_CALIB, 0, 0, 0);
            hub_display_off();
            break;
        case CAM_COLOR_CALIB:
            act_tsk(LUMP_CAM_COLOR_CALIB_TSK);
            break;
        case CAM_POS_CALIB:
            act_tsk(LUMP_CAM_POS_CALIB_TSK);
            break;
        case COLOR_COLOR_CALIB:
            act_tsk(LUMP_COLOR_SENSOR_COLOR_CALIB_TSK);
            break;
        default:
            break;
        }

        lump_button_wait_release(LUMP_BTN_BT);
    }
}