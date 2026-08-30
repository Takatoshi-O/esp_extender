#pragma once

#include <stdlib.h>
#include <stdbool.h>

#include "lump_sensors.h"

/**
 * @brief カメラの動作モードを表します。
 */
typedef enum {
    CAMERA_MODE_SYSTEM      = 0,
    CAMERA_MODE_POS_COLOR   = 1,
    CAMERA_MODE_12POS_COLOR = 2,
} camera_mode_t;

/**
 * @brief カメラから取得したYUV値を格納する構造体です。
 *
 * 各成分は16bit整数で保持します。
 */
typedef struct 
{
    int16_t y, u, v;
} lump_camera_yuv_t;

/**
 * @brief カメラキャリブレーションを開始するための処理です。
 */
void lump_camera_sta_calib();

/**
 * @brief カメラ機能を初期化します。
 *
 * カメラ関連の内部状態やコマンド処理を利用可能な状態にします。
 */
void lump_camera_init();

/**
 * @brief 指定したカメラインスタンスの設定を行います。
 * @param instanceID 設定対象のカメラインスタンスIDです。
 */
void lump_camera_setup(uint8_t instanceID);

/**
 * @brief 指定位置周辺から検出した色IDを取得します。
 * @param instanceID 使用するカメラインスタンスIDです。
 * @param x サンプリング中心のX座標です。
 * @param y サンプリング中心のY座標です。
 * @param radius サンプリング範囲の半径です。
 * @return 検出した色のIDです。
 */
lump_color_id_t lump_camera_get_color(uint8_t instanceID, int16_t x, int16_t y, uint8_t radius);

/**
 * @brief 指定した座標のYUV値を取得します。
 * @param instanceID 使用するカメラインスタンスIDです。
 * @param x 取得位置のX座標です。
 * @param y 取得位置のY座標です。
 * @return 取得したYUV値です。
 */
lump_camera_yuv_t lump_camera_get_yuv(uint8_t instanceID, int16_t x, int16_t y);

/**
 * @brief 12点分の色IDをまとめて取得します。
 * @param color_ids 取得した12個の色IDを書き込む配列です。要素数は12です。
 * @param instanceID 使用するカメラインスタンスIDです。
 * @param use_datalist 使用するデータリストを指定する値です。
 * @param radius 各点で使用するサンプリング範囲の半径です。
 * @return true 12点の色取得に成功した場合。
 * @return false 取得できなかった場合。
 */
bool lump_camera_get_12pos_color(lump_color_id_t color_ids[12], uint8_t instanceID, 
                                    uint8_t use_datalist, uint8_t radius);