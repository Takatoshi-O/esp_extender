#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
#include "header/lump_dispatch.h"

#include "kernel_cfg.h"

pup_device_t *esp32_dev;

/* ===================== 送信待機バッファ(ラウンドロビン, 16個) ===================== */

#define LUMP_SEND_BUF_CAPACITY 16

typedef struct {
    pup_device_t *dev;
    int32_t values[LUMP_PAYLOAD_LEN];
} lump_send_entry_t;

static lump_send_entry_t s_send_buf[LUMP_SEND_BUF_CAPACITY];
static uint8_t s_send_head = 0;   /* 次に書き込む位置 */
static uint8_t s_send_read = 0;   /* 次に送信する(最古の)位置 */
static uint8_t s_send_count = 0;  /* バッファ内の未送信件数 */

/* send_command()から呼ばれる。満杯の場合は最古のエントリを上書きする
 * (ラウンドロビン)。100Hzの送信サイクルハンドラと同時に触られるため
 * loc_cpu/unl_cpuで保護する */
static void lump_send_buf_push(pup_device_t *dev, const int32_t values[LUMP_PAYLOAD_LEN]) 
{
    loc_cpu();

    s_send_buf[s_send_head].dev = dev;
    memcpy(s_send_buf[s_send_head].values, values, sizeof(int32_t) * LUMP_PAYLOAD_LEN);
    s_send_head = (uint8_t)((s_send_head + 1) % LUMP_SEND_BUF_CAPACITY);

    if (s_send_count < LUMP_SEND_BUF_CAPACITY) 
    {
        s_send_count++;
    } 
    else 
    {
        /* 満杯だったので最古のエントリを上書きした
         * -> 読み出し位置もラウンドロビンで一つ進める */
        s_send_read = (uint8_t)((s_send_read + 1) % LUMP_SEND_BUF_CAPACITY);
    }

    unl_cpu();
}

/* 送信サイクルハンドラから呼ばれる。送るべきデータがあれば true を返す */
static bool lump_send_buf_pop(pup_device_t **out_dev, int32_t out_values[LUMP_PAYLOAD_LEN]) 
{
    bool ok = false;

    loc_cpu();
    if (s_send_count > 0) 
    {
        *out_dev = s_send_buf[s_send_read].dev;
        memcpy(out_values, s_send_buf[s_send_read].values, sizeof(int32_t) * LUMP_PAYLOAD_LEN);
        s_send_read = (uint8_t)((s_send_read + 1) % LUMP_SEND_BUF_CAPACITY);
        s_send_count--;
        ok = true;
    }
    unl_cpu();

    return ok;
}

/*
 * 送信サイクルハンドラ(100Hz = 10ms周期)。
 * バッファに送るべきデータがなければ何もしない(pup_device_set_valuesは呼ばない)。
 */
void lump_send_task(intptr_t exinf)
{
    pup_device_t *dev;
    int32_t values[LUMP_PAYLOAD_LEN];

    if (!lump_send_buf_pop(&dev, values)) {
        return;
    }

    pbio_error_t err = pup_device_set_values(dev, ESP32_MODE, values, LUMP_PAYLOAD_LEN);
    if (err != PBIO_SUCCESS) {
        syslog(LOG_ERROR, "コマンド送信エラー: %d", (int)err);
    }
}

void device_init(pbio_port_id_t port)
{
    lump_command_init();
    esp32_dev = pup_device_get_device(port, PBIO_IODEV_TYPE_ID_LUMP_UART);
    sta_cyc(LUMP_RECEIVE_CYC);
    sta_cyc(LUMP_SEND_CYC);
    act_tsk(LUMP_DISPATCH_TSK);
}

/* SPIKE側からESP32へコマンドを送る(必要な時だけ呼ぶ)。
 * ここでは即座には送信せず、送信待機バッファに積むだけにする。
 * 実際の pup_device_set_values() 呼び出しは lump_send_cyclic_handler が
 * 100Hzで巡回して行う */
void send_command(pup_device_t *dev, lump_sensor_type_t target, uint8_t command, uint8_t instanceID, int16_t p1, int16_t p2, int16_t p3, int16_t p4) 
{
    int32_t values[LUMP_PAYLOAD_LEN];
    values[0] = 0; /* コマンド側のシーケンス番号が必要ならここで管理する */
    values[1] = (int32_t)(((target & 0x07) << 5) | (command & 0x1F));
    values[2] = instanceID; 
    values[3] = (uint8_t)(p1 & 0xFF);
    values[4] = (uint8_t)((p1 >> 8) & 0xFF);
    values[5] = (uint8_t)(p2 & 0xFF);
    values[6] = (uint8_t)((p2 >> 8) & 0xFF);
    values[7] = (uint8_t)(p3 & 0xFF);
    values[8] = (uint8_t)((p3 >> 8) & 0xFF);
    values[9] = (uint8_t)(p4 & 0xFF);
    values[10] = (uint8_t)((p4 >> 8) & 0xFF);

    lump_send_buf_push(dev, values);
}