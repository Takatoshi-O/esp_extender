#pragma once

#include <stdlib.h>
#include "lump_comm.h"

/**
 * @brief 受信コマンドキューが保持できる最大件数です。
 */
#define LUMP_COMMAND_QUEUE_CAPACITY 16

/**
 * @brief 受信したコマンドを格納するエントリです。
 */
typedef struct {
    lump_sensor_type_t type;
    uint8_t command;      /* byte0下位5bit */
    uint8_t seq;
    uint8_t instance_id;
    int16_t v1, v2, v3, v4;
} lump_command_entry_t;

/**
 * @brief コマンド受信キューを初期化します。
 */
void lump_command_init(void);

/**
 * @brief 受信キューから1件のコマンドを取り出します。
 * @param out 取り出したコマンドを書き込む出力先です。
 * @return true コマンドを取得できた場合。
 * @return false キューが空の場合。
 */
bool lump_command_pop(lump_command_entry_t *out);

/**
 * @brief 生の受信データをコマンドキューへ追加します。
 * @param raw LUMP_PAYLOAD_LEN個の値からなる受信データです。
 */
void lump_command_push(const int32_t raw[LUMP_PAYLOAD_LEN]);