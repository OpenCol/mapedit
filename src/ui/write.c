/* write.c -- writing text: building strings from words, drawing them in the menu font.
 *
 * Two string builders append to a caller's buffer, which must have room: space_string a space,
 * say_string a word from the string store by index. Two writers draw a string into the work
 * screen (scr_work) in the small menu font: write_string left-aligned in the basic colour,
 * write_right right-aligned in a colour given. The writers do not interpret the popup markup
 * ({ } and ~); only popups do.
 */
#include "msc.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as the 1994 compiler did */
#include "menu.h"
#include "pal.h"
#include "popup.h"
#include "theme.h"
#include "font.h"
#include "loader.h"
#include "map.h"
#include "mapedit.h"
#include "records.h"
#include "screen.h"
#include "text.h"
#include "units.h"
#include "w16types.h"
/* Appends a space to buf. */
void space_string(char *buf)
{
    msc_strcat(buf, " ");
}
/* Appends string number `word` from the string store. */
void say_string(char *buf, i16 word)
{
    msc__fstrcat(buf, strings(word));
}
/* Writes s at (x, y) in the menu font and the basic colour; returns the x after it. */
i16 write_string(char *s, register i16 x, i16 y)
{
    font_set_colors(-1, basic_color, 0, 0);
    return font_write(&scr_work, x, y, menu_font, s, 0);
}
/* Writes s in the menu font in `color`, ending at x; returns the x it starts at. */
i16 write_right(char *s, i16 x, i16 y, i16 color)
{
    font_set_colors(-1, color, color, color);
    x -= font_string_width(0, menu_font, s);
    font_write(&scr_work, x, y, menu_font, s, 0);
    return x;
}

