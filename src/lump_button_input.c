#include <string.h>
#include <stddef.h>
#include <include/kernel.h>
#include "pbio/control.h"

#include "header/lump_button_input.h"

#define POLL_INTERVAL_US (50 * 1000)
#define POLL_INTERVAL_MS (50)

/* ===== 状態管理タスク(lump_button_task)だけが読み書きする内部状態 ===== */

static const hub_button_t s_mask[LUMP_BTN_MAX] = {
    [LUMP_BTN_LEFT]   = HUB_BUTTON_LEFT,
    [LUMP_BTN_RIGHT]  = HUB_BUTTON_RIGHT,
    [LUMP_BTN_CENTER] = HUB_BUTTON_CENTER,
    [LUMP_BTN_BT]     = HUB_BUTTON_BT,
};

/* 長押しのしきい値はグローバル設定。detect_edge() から読み取り専用で参照される。 */
static uint32_t s_long_threshold_ms[LUMP_BTN_MAX] = {
    [LUMP_BTN_LEFT]   = 1200,
    [LUMP_BTN_RIGHT]  = 1200,
    [LUMP_BTN_CENTER] = 1200,
    [LUMP_BTN_BT]     = 1000,
};

static bool     s_prev_held[LUMP_BTN_MAX];
static uint32_t s_held_ms[LUMP_BTN_MAX];

/* ===== 他タスクと共有するデータ(loc_cpu/unl_cpuで保護してアクセス) =====
 * lump_button_t の値でそのままインデックスする（[0]=NONEは未使用）。
 */
static lump_button_status_t s_status_cache[LUMP_BTN_MAX];

void lump_button_set_long_threshold(lump_button_t button, uint32_t threshold_ms)
{
    if (button > LUMP_BTN_NONE && button < LUMP_BTN_MAX)
    {
        s_long_threshold_ms[button] = threshold_ms;
    }
}

/*
 * 1ティック分、押下状態と held_ms を更新して共有キャッシュへ書き込むだけ。
 * 短押し/長押しの判定は行わない(各呼び出し元が detect_edge で行う)。
 * lump_button_task() の中からのみ呼ばれる。
 */
static void scan_tick(hub_button_t bits)
{
    lump_button_status_t local_status[LUMP_BTN_MAX] = {0};

    for (lump_button_t b = LUMP_BTN_LEFT; b < LUMP_BTN_MAX; b++)
    {
        bool now = (bits & s_mask[b]) != 0;
        bool was = s_prev_held[b];

        if (now && !was)
        {
            s_held_ms[b] = 0;
        }
        else if (now && was)
        {
            s_held_ms[b] += POLL_INTERVAL_MS;
        }
        else
        {
            s_held_ms[b] = 0;
        }
        s_prev_held[b] = now;

        local_status[b].button  = b;
        local_status[b].pressed = now;
        local_status[b].held_ms = now ? s_held_ms[b] : 0;
    }

    loc_cpu();
    memcpy(s_status_cache, local_status, sizeof(local_status));
    unl_cpu();
}

void lump_button_task(intptr_t exinf)
{
    while (true)
    {
        hub_button_t bits;
        hub_button_is_pressed(&bits);
        scan_tick(bits);
        dly_tsk(POLL_INTERVAL_US);
    }
}

bool lump_button_query_status(lump_button_status_t out[LUMP_BTN_MAX])
{
    bool any_pressed = false;

    loc_cpu();
    memcpy(out, s_status_cache, sizeof(s_status_cache));
    unl_cpu();

    for (lump_button_t b = LUMP_BTN_LEFT; b < LUMP_BTN_MAX; b++)
    {
        if (out[b].pressed) any_pressed = true;
    }
    return any_pressed;
}

bool lump_button_detect_edge(const lump_button_status_t status[LUMP_BTN_MAX],
                              lump_button_t button,
                              lump_button_edge_state_t *state,
                              lump_button_event_t *out_event)
{
    bool now  = status[button].pressed;
    bool was  = state->was_pressed;
    bool fired = false;

    if (now)
    {
        state->last_held_ms = status[button].held_ms;

        if (!was)
        {
            /* 新規押下 */
            state->long_fired = false;
        }
        else if (!state->long_fired && state->last_held_ms >= s_long_threshold_ms[button])
        {
            /* 押され続けたまま、しきい値に到達した瞬間だけ1回発火 */
            state->long_fired = true;
            *out_event = (lump_button_event_t){
                .button  = button,
                .kind    = LUMP_PRESS_LONG,
                .held_ms = state->last_held_ms,
            };
            fired = true;
        }
    }
    else if (was)
    {
        /* 離された瞬間。長押しが未確定なら短押しとして確定させる */
        if (!state->long_fired)
        {
            *out_event = (lump_button_event_t){
                .button  = button,
                .kind    = LUMP_PRESS_SHORT,
                .held_ms = state->last_held_ms,
            };
            fired = true;
        }
        state->long_fired = false;
    }

    state->was_pressed = now;
    return fired;
}

void lump_button_wait_release(lump_button_t button)
{
    lump_button_status_t status[LUMP_BTN_MAX];
    do
    {
        lump_button_query_status(status);
        if (status[button].pressed) dly_tsk(POLL_INTERVAL_US);
    } while (status[button].pressed);
}