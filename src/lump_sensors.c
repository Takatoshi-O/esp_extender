#include <stdlib.h>
#include <stdio.h>
#include <include/kernel.h>
#include <t_syslog.h>
#include <string.h>

#include "lump_sensors.h"

#include "lump_color.h"

void lump_sensors_init()
{
    lump_color_init();
}

char lump_color_id_to_char(lump_color_id_t color)
{
    switch (color) {
    case LUMP_COLOR_ERROR:
        return 'E';

    case LUMP_COLOR_BLACK:
        return 'B';

    case LUMP_COLOR_WHITE:
        return 'W';

    case LUMP_COLOR_RED:
        return 'R';

    case LUMP_COLOR_GREEN:
        return 'G';

    case LUMP_COLOR_BLUE:
        return 'C';

    case LUMP_COLOR_YELLOW:
        return 'Y';

    case LUMP_COLOR_ORANGE:
        return 'O';

    case LUMP_COLOR_PURPLE:
        return 'P';

    case LUMP_COLOR_CYAN:
        return 'C';

    case LUMP_COLOR_MAGENTA:
        return 'M';

    case LUMP_COLOR_BROWN:
        return 'B';

    case LUMP_COLOR_GRAY:
        return 'G';

    case LUMP_COLOR_PINK:
        return 'I';

    case LUMP_COLOR_LIME:
        return 'L';

    case LUMP_COLOR_NAVY:
        return 'V';

    case LUMP_COLOR_UNKNOWN:
        return '?';

    default:
        return '?';
    }
}