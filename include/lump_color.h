#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "lump_sensors.h"

/*
 * カラーセンサーのモード定義。
 * 0: 初期設定用
 * 1: RGB値
 * 2: 色ID
 */
/**
 * @brief カラーセンサーの動作モードを表します。
 */
typedef enum {
    COLOR_MODE_SYSTEM = 0,
    COLOR_MODE_RGBC = 1,
    COLOR_MODE_COLOR_ID = 2,
    COLOR_MODE_NOTIFY_COLOR = 3,
} lump_color_sensor_mode_t;

/**
 * @brief カラーセンサーから取得したRGBC値を格納します。
 *
 * R/G/Bは各色成分、CはClear成分を表します。
 */
typedef struct 
{
    int16_t r, g, b, c;
} lump_color_rgbc_t;

/**
 * @brief カラーセンサー機能を初期化します。
 */
void lump_color_init();

/**
 * @brief 指定したカラーセンサーインスタンスの設定を行います。
 * @param instanceID 設定対象のインスタンスIDです。
 */
void lump_color_setup(uint8_t instanceID);

/**
 * @brief 指定したカラーセンサーが現在認識している色IDを取得します。
 * @param instanceID 使用するカラーセンサーのインスタンスIDです。
 * @return 認識した色のIDです。
 */
lump_color_id_t lump_color_get_color(uint8_t instanceID);

/**
 * @brief 指定したカラーセンサーの最新RGBC値を取得します。
 * @param instanceID 使用するカラーセンサーのインスタンスIDです。
 * @return 取得したRGBC値です。
 */
lump_color_rgbc_t lump_color_get_rgbc(uint8_t instanceID);

/**
 * @brief 指定したカラーセンサーに設定された通知用の色IDを取得します。
 * @param instanceID 使用するカラーセンサーのインスタンスIDです。
 * @return 通知用の色IDです。
 */
lump_color_id_t lump_color_get_notify_color(uint8_t instanceID);

/**
 * @brief 指定した色に対するカラーセンサーのキャリブレーションを要求します。
 * @param instanceID 対象のカラーセンサーインスタンスIDです。
 * @param colorID キャリブレーション対象の色IDです。
 */
void lump_color_request_calib(uint8_t instanceID, lump_color_id_t colorID);

/**
 * @brief 通知対象の色IDを追加または削除します。
 * @param instanceID 対象のカラーセンサーインスタンスIDです。
 * @param colorID 追加・削除する色IDです。
 * @param isadd trueで追加、falseで削除します。
 */
void lump_color_set_notify_color(uint8_t instanceID, lump_color_id_t colorID, bool isadd);