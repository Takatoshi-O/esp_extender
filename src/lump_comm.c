#include <stdlib.h>
#include <stdio.h>
#include <include/kernel.h>
#include <t_syslog.h>

#include <spike/hub/system.h>
#include "lump_comm.h"
#include "lump_comm_tsk.h"

#include "spike/pup_device.h"
#include "spike/cb_error.h"

#include "spike/hub/button.h"
#include "spike/hub/display.h"
#include "spike/hub/imu.h"
#include "spike/hub/light.h"

#include <pbio/color.h>

#include "header/lump_receive.h"

#include "kernel_cfg.h"


pup_device_t *esp32_dev;

/* センサー種別ごとに、直近に処理したシーケンス番号を覚えておく */
static uint8_t last_seq[SENSOR_TYPE_MAX] = {0};
static bool first_read[SENSOR_TYPE_MAX] = {true, true, true, true, true, true, true, true};


/* リトルエンディアン2バイトから符号付き16bit整数へ変換 */
static int16_t decode_int16_le(const int32_t *raw, int byte_offset) 
{
    uint8_t lo = (uint8_t)raw[byte_offset];
    uint8_t hi = (uint8_t)raw[byte_offset + 1];
    return (int16_t)((hi << 8) | lo);
}

void device_init(pbio_port_id_t port)
{
    esp32_dev = pup_device_get_device(port, PBIO_IODEV_TYPE_ID_LUMP_UART);
}

/* SPIKE側からESP32へコマンドを送る(必要な時だけ呼ぶ) */
void send_command(pup_device_t *dev, lump_sensor_type_t target, uint8_t command, uint8_t instanceID, int16_t p1, int16_t p2, int16_t p3, int16_t p4) 
{
    int32_t values[LUMP_PAYLOAD_LEN];
    values[0] = (int32_t)(((target & 0x07) << 5) | (command & 0x1F));
    values[1] = instanceID; 
    values[2] = 0; /* コマンド側のシーケンス番号が必要ならここで管理する */
    values[3] = (uint8_t)(p1 & 0xFF);
    values[4] = (uint8_t)((p1 >> 8) & 0xFF);
    values[5] = (uint8_t)(p2 & 0xFF);
    values[6] = (uint8_t)((p2 >> 8) & 0xFF);
    values[7] = (uint8_t)(p3 & 0xFF);
    values[8] = (uint8_t)((p3 >> 8) & 0xFF);
    values[9] = (uint8_t)(p4 & 0xFF);
    values[10] = (uint8_t)((p4 >> 8) & 0xFF);

    pbio_error_t err = pup_device_set_values(dev, ESP32_MODE, values, LUMP_PAYLOAD_LEN);
    if (err != PBIO_SUCCESS) {
        syslog(LOG_ERROR, "コマンド送信エラー: %d", (int)err);
    }
}

