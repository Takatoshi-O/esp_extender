#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "lump_receive.h"

/*
 * コマンドを受け取った時に呼ばれるハンドラの型。
 */
/**
 * @brief 受信コマンドを処理するハンドラ関数の型です。
 * @param data 受信したコマンドエントリです。
 */
typedef void (*lump_command_handler_t)(lump_command_entry_t data);

/*
 * センサー種別ごとにハンドラを1つ登録する。
 * 同じ種別に対して再度呼ぶと、以前のハンドラを上書きする。
 * handler に NULL を渡すと、その種別の登録を解除する。
 */
/**
 * @brief センサー種別に対応するコマンドハンドラを登録します。
 * @param type ハンドラを登録するセンサー種別です。
 * @param handler 登録するハンドラです。NULLを指定すると登録を解除します。
 */
void lump_command_dispatch_register(lump_sensor_type_t type, lump_command_handler_t handler);