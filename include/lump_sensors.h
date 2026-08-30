#pragma once

//Color ID
/**
 * @brief センサーで扱う色の識別子です。
 *
 * UNKNOWNは未判定、ERRORは色判定時のエラーを表します。
 */
typedef enum {
    LUMP_COLOR_ERROR = -1,
    LUMP_COLOR_UNKNOWN = 0,
    LUMP_COLOR_BLACK,
    LUMP_COLOR_WHITE,
    LUMP_COLOR_RED,
    LUMP_COLOR_GREEN,
    LUMP_COLOR_BLUE,
    LUMP_COLOR_YELLOW,
    LUMP_COLOR_ORANGE,
    LUMP_COLOR_PURPLE,
    LUMP_COLOR_CYAN,
    LUMP_COLOR_MAGENTA,
    LUMP_COLOR_BROWN,
    LUMP_COLOR_GRAY,
    LUMP_COLOR_PINK,
    LUMP_COLOR_LIME,
    LUMP_COLOR_NAVY,
} lump_color_id_t;

/**
 * @brief 1種類のセンサーで扱える最大インスタンス数です。
 */
#define LUMP_MAX_INSTANCES_PER_TYPE 8

/**
 * @brief 色IDを表示用の文字に変換します。
 * @param color 変換対象の色IDです。
 * @return 色IDに対応する文字です。
 */
char lump_color_id_to_char(lump_color_id_t color);