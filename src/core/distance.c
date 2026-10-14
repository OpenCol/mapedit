/* distance.c -- minimax, clamping a value to a range. The module is shared with the game.
 */
#include "msc.h"
#pragma pack(2)   /* 2-byte record packing, as in 1994: the file formats depend on it */
#include "font.h"
#include "menu.h"
#include "pal.h"
#include "popup.h"
#include "loader.h"
#include "mapedit.h"
#include "records.h"
#include "units.h"

/* value clamped to lo..hi; hi wins if lo > hi. */
i16 minimax(i16 value, i16 lo, i16 hi)
{
    value = max(value, lo);
    value = min(value, hi);
    return value;
}

