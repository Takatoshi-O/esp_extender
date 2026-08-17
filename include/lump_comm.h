#pragma once

#include <stdlib.h>
#include "spike/pup_device.h"

#define ESP32_PORT       PBIO_PORT_ID_A   /* 実際に接続したポートに合わせて変更 */
#define ESP32_MODE       0
#define LUMP_PAYLOAD_LEN 11

extern pup_device_t *esp32_dev;

/* ESP32側 lump_device.h の lump_sensor_type_t と対応させる */
typedef enum {
    SENSOR_SYSTEM  = 0,
    SENSOR_COLOR   = 1,
    SENSOR_CAMERA  = 2,
    SENSOR_TYPE_3  = 3,
    SENSOR_TYPE_4  = 4,
    SENSOR_TYPE_5  = 5,
    SENSOR_TYPE_6  = 6,
    SENSOR_TYPE_7  = 7,
    SENSOR_TYPE_MAX,
} lump_sensor_type_t;

void device_init(pbio_port_id_t port);

void send_command(pup_device_t *dev, lump_sensor_type_t target, uint8_t command, uint8_t instanceID, int16_t p1, int16_t p2, int16_t p3, int16_t p4);
