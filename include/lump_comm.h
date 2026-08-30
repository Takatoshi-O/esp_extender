#pragma once

#include <stdlib.h>
#include "spike/pup_device.h"

/**
 * @brief ESP32を接続するPUPポートです。
 */
#define ESP32_PORT       PBIO_PORT_ID_A   /* 実際に接続したポートに合わせて変更 */
/**
 * @brief ESP32との通信で使用するモード値です。
 */
#define ESP32_MODE       0
/**
 * @brief 1コマンドで扱うペイロード要素数です。
 */
#define LUMP_PAYLOAD_LEN 11

/**
 * @brief ESP32側デバイスへの接続を表すPUPデバイスポインタです。
 */
extern pup_device_t *esp32_dev;

/* ESP32側 lump_device.h の lump_sensor_type_t と対応させる */
/**
 * @brief 通信先のセンサー種別を表します。
 *
 * ESP32側で定義されているセンサー種別と対応させて使用します。
 */
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

/**
 * @brief 指定したSPIKEポート上のESP32デバイス通信を初期化します。
 * @param port ESP32を接続したPUPポートです。
 */
void device_init(pbio_port_id_t port);

/**
 * @brief ESP32側センサーへコマンドを送信します。
 * @param dev 送信先のPUPデバイスです。
 * @param target 対象センサーの種類です。
 * @param command 実行するコマンド番号です。
 * @param instanceID 対象インスタンスのIDです。
 * @param p1 コマンドパラメータ1です。
 * @param p2 コマンドパラメータ2です。
 * @param p3 コマンドパラメータ3です。
 * @param p4 コマンドパラメータ4です。
 */
void send_command(pup_device_t *dev, lump_sensor_type_t target, uint8_t command, uint8_t instanceID, int16_t p1, int16_t p2, int16_t p3, int16_t p4);
