#pragma once

#include <include/kernel.h>

/*
 *  タスクの優先度の定義(min = 5)
 */

#define LUMP_RECEIVE_PRIORITY 5
#define LUMP_DISPATCH_PRIORITY 7
#define LUMP_CAMERA_CALIB_PRIORITY 8

#ifndef STACK_SIZE
#define	STACK_SIZE		4096		/* タスクのスタックサイズ */
#endif /* STACK_SIZE */

/*
 *  関数のプロトタイプ宣言
 */

extern void receive_command(intptr_t exinf);
extern void lump_dispatch_poll(intptr_t exinf);
extern void lump_camera_calib(intptr_t exinf);
extern void lump_change_cam_calib_mode(intptr_t exinf);