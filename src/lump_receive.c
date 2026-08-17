#include "header/lump_receive.h"
#include <stdlib.h>
#include <stdio.h>
#include <include/kernel.h>
#include <t_syslog.h>
#include <t_stddef.h>

#include "lump_comm_tsk.h"

#include "kernel_cfg.h"

#define ACK_MESSAGE 0x00

static lump_command_entry_t s_queue[LUMP_COMMAND_QUEUE_CAPACITY];
static size_t s_head = 0;   /* 次に書き込む位置(最新) */
static size_t s_tail = 0;   /* 次に取り出す位置(最古) */
static size_t s_count = 0;  /* 現在の件数 */

void lump_command_init(void) {
    s_head = 0;
    s_tail = 0;
    s_count = 0;
}

void lump_command_push(const uint8_t raw[LUMP_PAYLOAD_LEN]) {
    lump_command_entry_t entry;
    entry.type        = (lump_sensor_type_t)((raw[0] >> 5) & 0x07);
    entry.command      = raw[0] & 0x1F;
    entry.seq          = raw[1]; /* コマンド用パケットはbyte1=シーケンス番号 */
    entry.instance_id  = raw[2]; /* コマンド用パケットはbyte2=インスタンスID */

    uint8_t lo, hi;
    lo = raw[3];  hi = raw[4];  entry.v1 = (int16_t)((hi << 8) | lo);
    lo = raw[5];  hi = raw[6];  entry.v2 = (int16_t)((hi << 8) | lo);
    lo = raw[7];  hi = raw[8];  entry.v3 = (int16_t)((hi << 8) | lo);
    lo = raw[9];  hi = raw[10]; entry.v4 = (int16_t)((hi << 8) | lo);

    if (loc_mtx(LUMP_RECEIVE_MTX) == E_OK) {
        s_queue[s_head] = entry;
        s_head = (s_head + 1) % LUMP_COMMAND_QUEUE_CAPACITY;

        if (s_count < LUMP_COMMAND_QUEUE_CAPACITY) {
            s_count++;
        } else {
            /* 満杯なので、最も古いものを1つ捨てる(tailを進める) */
            s_tail = (s_tail + 1) % LUMP_COMMAND_QUEUE_CAPACITY;
        }
        unl_mtx(LUMP_RECEIVE_MTX);
    }
}

bool lump_command_pop(lump_command_entry_t *out) {
    bool ok = false;
    if (loc_mtx(LUMP_RECEIVE_MTX) == E_OK) {
        if (s_count > 0) {
            *out = s_queue[s_tail];
            s_tail = (s_tail + 1) % LUMP_COMMAND_QUEUE_CAPACITY;
            s_count--;
            ok = true;
        }
        unl_mtx(LUMP_RECEIVE_MTX);
    }
    return ok;
}

size_t lump_command_count(void) {
    size_t c = 0;
    if (loc_mtx(LUMP_RECEIVE_MTX) == E_OK) {
        c = s_count;
        unl_mtx(LUMP_RECEIVE_MTX);
    }
    return c;
}

void receive_command(intptr_t exinf)
{
    uint8_t raw[LUMP_PAYLOAD_LEN];
    pbio_error_t err = pup_device_get_values(esp32_dev, ESP32_MODE, (int32_t*)raw);

    if (err == PBIO_SUCCESS) {
        if (raw[0] != ACK_MESSAGE)
        {
            lump_command_push(raw);
        }
    } else {
        syslog(LOG_ERROR, "受信エラー: %d", (int)err);
    }
}
