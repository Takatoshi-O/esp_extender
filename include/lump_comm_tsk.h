#pragma once

#include <include/kernel.h>

/*
 *  タスクの優先度の定義(min = 5)
 */

/**
 * @brief コマンド送信タスクの優先度です。
 */
#define LUMP_SEND_PRIORITY 5
/**
 * @brief コマンド受信タスクの優先度です。
 */
#define LUMP_RECEIVE_PRIORITY 5
/**
 * @brief コマンド配送タスクの優先度です。
 */
#define LUMP_DISPATCH_PRIORITY 7
/**
 * @brief カメラ・キャリブレーション関連タスクの優先度です。
 */
#define LUMP_CAMERA_CALIB_PRIORITY 8

#ifndef STACK_SIZE
/**
 * @brief タスクに割り当てる既定のスタックサイズです。
 */
#define	STACK_SIZE		4096		/* タスクのスタックサイズ */
#endif /* STACK_SIZE */

/*
 *  関数のプロトタイプ宣言
 */

/**
 * @brief コマンド送信用タスクのエントリポイントです。
 * @param exinf TOPPERS/RTOSから渡される拡張情報です。
 */
extern void lump_send_task(intptr_t exinf);
/**
 * @brief 受信コマンド処理タスクのエントリポイントです。
 * @param exinf TOPPERS/RTOSから渡される拡張情報です。
 */
extern void receive_command(intptr_t exinf);
/**
 * @brief 受信したコマンドを登録済みハンドラへ配送するタスクのエントリポイントです。
 * @param exinf TOPPERS/RTOSから渡される拡張情報です。
 */
extern void lump_dispatch_poll(intptr_t exinf);

/**
 * @brief ハードウェアボタンの状態を定期的に取得するタスクです。
 * @param exinf TOPPERS/RTOSから渡される拡張情報です。
 */
extern void lump_button_task(intptr_t exinf);
/**
 * @brief キャリブレーションモード変更を処理するタスクのエントリポイントです。
 * @param exinf TOPPERS/RTOSから渡される拡張情報です。
 */
extern void change_calib_mode(intptr_t exinf);
/**
 * @brief カメラ位置キャリブレーションを処理するタスクです。
 * @param exinf TOPPERS/RTOSから渡される拡張情報です。
 */
extern void cam_pos_calib(intptr_t exinf);
/**
 * @brief カメラ色キャリブレーションを処理するタスクです。
 * @param exinf TOPPERS/RTOSから渡される拡張情報です。
 */
extern void cam_color_calib(intptr_t exinf);
/**
 * @brief カラーセンサー色キャリブレーションを処理するタスクです。
 * @param exinf TOPPERS/RTOSから渡される拡張情報です。
 */
extern void color_sensor_color_calib(intptr_t exinf);