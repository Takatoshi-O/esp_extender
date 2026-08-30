#ifndef LUMP_BUTTON_INPUT_H
#define LUMP_BUTTON_INPUT_H

#include <stdint.h>
#include <stdbool.h>
#include "spike/hub/button.h"

/*
 * ハードウェアのスキャン(hub_button_is_pressed)は lump_button_task() だけが行う。
 * 他のタスクはそこから読み取り専用で状態を取得し、
 * 短押し/長押しの判定は「呼び出し側が自分専用に持つ状態」を使って
 * 自分自身で行う(lump_button_detect_edge)。
 *
 * 以前のバージョンは短押し/長押しの確定を単一の共有イベントキューに積み、
 * 複数タスクがそこから取り合う(pop)方式だったが、
 * 「自分宛てでないイベントを黙って捨てる」実装だったため、
 * どのタスクが先にキューを取るかによってボタンが反応しないことがあった。
 * この設計はその問題を構造的に無くすためのもの:
 * イベントは「消費されて無くなる」ものではなく、
 * 各タスクが毎ティック自分の目線で状態遷移を見て判定するだけなので、
 * 複数タスクが同じ物理ボタンを別々の目的で監視しても取り合いが起きない。
 */

/**
 * @brief 物理ボタンを識別する列挙型です。
 */
typedef enum
{
    LUMP_BTN_NONE = -1,
    LUMP_BTN_LEFT,
    LUMP_BTN_RIGHT,
    LUMP_BTN_CENTER,
    LUMP_BTN_BT,
    LUMP_BTN_MAX,
} lump_button_t;

/**
 * @brief ボタン入力の押下種別です。
 */
typedef enum
{
    LUMP_PRESS_SHORT,
    LUMP_PRESS_LONG,
} lump_press_kind_t;

/**
 * @brief 確定したボタンイベントを表します。
 */
typedef struct
{
    lump_button_t     button;
    lump_press_kind_t kind;
    uint32_t          held_ms;   /* 確定した時点での保持時間(ms) */
} lump_button_event_t;

/**
 * @brief 各ボタンの現在状態を表します。
 */
typedef struct
{
    lump_button_t button;
    bool          pressed;    /* 現在押されているか */
    uint32_t      held_ms;    /* 押されてからの経過時間(ms)。pressed=falseなら0 */
} lump_button_status_t;

/*
 * lump_button_detect_edge() の呼び出し側ごとに1つ持つ状態。
 * 同じボタンを監視する呼び出し元が複数あってもよい
 * （例: BT長押しを見るタスクとBT短押しを見るタスクが同時に存在してよい）。
 * その場合は呼び出し元ごとに別々のインスタンスを用意すること。
 * 初期値はすべて0でよい（グローバル/static変数なら初期化不要、
 * ローカル変数の場合は `= {0}` で初期化する）。
 */
/**
 * @brief 呼び出し元ごとの短押し・長押しエッジ判定状態を保持します。
 *
 * 同じボタンを複数の呼び出し元で監視する場合は、それぞれ別の状態を用意します。
 */
typedef struct
{
    bool was_pressed;
    bool long_fired;
    uint32_t last_held_ms;
} lump_button_edge_state_t;

/* ボタンごとの長押し判定しきい値(ms)を変更する。未設定時は LEFT/RIGHT/CENTER=1200ms, BT=1000ms */
/**
 * @brief ボタンごとの長押し判定しきい値を設定します。
 * @param button しきい値を変更するボタンです。
 * @param threshold_ms 長押しと判定する保持時間(ms)です。
 */
void lump_button_set_long_threshold(lump_button_t button, uint32_t threshold_ms);

/*
 * 状態管理タスク本体。プログラム中でこのタスクだけが hub_button_is_pressed を呼ぶ。
 * 50ms周期の while ループでポーリングし、4ボタン分の現在の押下状態と
 * 押され続けている時間(held_ms)を内部キャッシュへ書き込むだけを行う。
 * 短押し/長押しの判定はここでは行わない（各呼び出し元が detect_edge で行う）。
 * .cfgファイルで1つのタスクとして登録して起動すること。
 */
/**
 * @brief ボタン状態を定期的にスキャンして内部キャッシュを更新するタスクです。
 * @param exinf RTOSから渡される拡張情報です。
 */
void lump_button_task(intptr_t exinf);

/*
 * 非ブロッキング。lump_button_task() が更新した最新の状態キャッシュを
 * out にコピーするだけで、ハードウェアの再スキャンは行わない。
 * out は lump_button_t の値でそのままインデックスできる
 * （out[LUMP_BTN_CENTER] のように書ける）。out[LUMP_BTN_NONE] は未使用。
 * out は要素数 LUMP_BTN_MAX の配列であること。
 * 戻り値: LEFT/RIGHT/CENTER/BT のいずれかが押されていれば true。
 */
/**
 * @brief 最新のボタン状態を出力配列へコピーします。
 * @param out ボタン状態を書き込むLUMP_BTN_MAX要素の配列です。
 * @return true いずれかのボタンが押されている場合。
 * @return false すべてのボタンが離されている場合。
 */
bool lump_button_query_status(lump_button_status_t out[LUMP_BTN_MAX]);

/*
 * 呼び出し側が保持する `state` と、直近の lump_button_query_status() の
 * 結果 `status` を突き合わせて、`button` の短押し/長押しが
 * 今回のティックで確定したかどうかを判定する（非ブロッキング、待たない）。
 *
 * 確定した場合は true を返し *out_event に内容を書き込む。
 * 確定していなければ false を返し *out_event は変更しない。
 *
 * 呼び出し側は毎ティック（自分のポーリングループのたびに）これを呼ぶこと。
 * `state` は呼び出し元がスタック上またはstaticで保持し、他の呼び出し元とは
 * 共有しないこと（各監視者が自分の目線で独立に判定するための設計）。
 */
/**
 * @brief 指定ボタンの短押し・長押しイベントを状態遷移から判定します。
 * @param status 最新の全ボタン状態です。
 * @param button 判定対象のボタンです。
 * @param state 呼び出し元が保持するエッジ判定状態です。
 * @param out_event イベント確定時に結果を書き込む出力先です。
 * @return true 今回の呼び出しでイベントが確定した場合。
 * @return false イベントが確定していない場合。
 */
bool lump_button_detect_edge(const lump_button_status_t status[LUMP_BTN_MAX],
                              lump_button_t button,
                              lump_button_edge_state_t *state,
                              lump_button_event_t *out_event);

/*
 * `button` が離されるまでブロックする（内部で lump_button_query_status を
 * 50ms間隔でポーリングするだけで、ハードウェアには触れない）。
 */
/**
 * @brief 指定したボタンが離されるまで待機します。
 * @param button 待機対象のボタンです。
 */
void lump_button_wait_release(lump_button_t button);

#endif /* LUMP_BUTTON_INPUT_H */