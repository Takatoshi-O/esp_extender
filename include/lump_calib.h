#pragma once

#include <stdbool.h>
#include <stdlib.h>

/**
 * @brief キャリブレーションモードが変更されたかを確認します。
 * @return true キャリブレーションモードの変更を検出した場合。
 * @return false 変更がない場合。
 */
bool is_change_calib_mode();

/**
 * @brief キャリブレーション機能の初期化・開始処理を実行します。
 *
 * キャリブレーション関連の状態を開始するために使用します。
 */
void lump_sta_calib();
