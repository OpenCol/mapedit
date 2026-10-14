/* write.c -- writing text: strings built from words and numbers, drawn in the editor's fonts.
 *
 * Two families. The string builders append to a caller's buffer, which must have room:
 * punctuation (space_string ... times_string), words from the string store by index
 * (say_string), and numbers (say_number, say_binary, say_long). The writers draw a string into
 * the work screen (scr_work): the plain ones in the small menu font, the "big" ones in the
 * interface font (FONTINTR), left-aligned, right-aligned or centred. The writers do not
 * interpret the popup markup ({ } and ~); only popups do. The editor itself uses only
 * write_string, write_right, space_string and say_string; the rest are kept as the original had
 * them.
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
/* The punctuation builders: each appends its string to buf. */
void space_string(char *buf)
{
    msc_strcat(buf, " ");
}
/* Appends n spaces. */
void spacer(char *buf, i16 n)
{
    i16 i;

    for (i = 0; i < n; i++)
        space_string(buf);
}
void comma_string(char *buf)
{
    msc_strcat(buf, ", ");
}
void colon_string(char *buf)
{
    msc_strcat(buf, ": ");
}
/* Appends a full stop and two spaces. */
void period_string(char *buf)
{
    msc_strcat(buf, ".  ");
}
void percent_string(char *buf)
{
    msc_strcat(buf, "%");
}
void open_string(char *buf)
{
    msc_strcat(buf, "(");
}
void close_string(char *buf)
{
    msc_strcat(buf, ")");
}
/* Appends the popup markup that starts (brace_on) or ends (brace_off) a highlighted span. */
void brace_on(char *buf)
{
    msc_strcat(buf, "{");
}
void brace_off(char *buf)
{
    msc_strcat(buf, "}");
}
void plus_string(char *buf)
{
    msc_strcat(buf, "+");
}
void minus_string(char *buf)
{
    msc_strcat(buf, "-");
}
/* Appends "x", as in 2x. */
void times_string(char *buf)
{
    msc_strcat(buf, "x");
}
/* Appends string number `word` from the string store. */
void say_string(char *buf, i16 word)
{
    msc__fstrcat(buf, strings(word));
}
/* The same, highlighted: wrapped in { }. */
void say_braced(char *buf, i16 word)
{
    brace_on(buf);
    msc__fstrcat(buf, strings(word));
    brace_off(buf);
}
/* Appends number in decimal. */
void say_number(char *s, i16 number)
{
    char buf[0x14];

    msc_itoa(number, buf, 10);
    msc__fstrcat(s, buf);
}
/* Appends number in binary, padded with zeros to 8 digits (a longer number is not cut). */
void say_binary(char *s, i16 number)
{
    char buf[0x14];
    i16 i;
    i16 len;

    msc_itoa(number, buf, 2);
    len = msc_strlen(buf);
    for (i = 0; i < 8 - len; i++)
        msc__fstrcat(s, "0");
    msc__fstrcat(s, buf);
}
/* Appends a 32-bit number in decimal. */
void say_long(char *s, i32 number)
{
    char buf[0x14];

    msc_ltoa(number, buf, 10);
    msc__fstrcat(s, buf);
}
/* Appends a 32-bit number followed by "$". */
void say_bucks(char *s, i32 number)
{
    say_long(s, number);
    msc__fstrcat(s, "$");
}
/* The width in pixels of s in the menu font, without the gap after the last character. */
i16 string_width(char *s)
{
    return font_string_width(0, menu_font, s) - 1;
}
/* The same in the interface font. */
i16 string_width_2(char *s)
{
    return font_string_width(0, font_inter, s) - 1;
}
/* Writes s at (x, y) in the menu font and the basic colour; returns the x after it. */
i16 write_string(char *s, register i16 x, i16 y)
{
    font_set_colors(-1, basic_color, 0, 0);
    return font_write(&scr_work, x, y, menu_font, s, 0);
}
/* Writes s at (x, y) in the menu font, all three glyph colours `color`; returns the x after
 * it. The `if (0)` is a no-op. */
i16 write_in_color(char *s, i16 x, i16 y, i16 color)
{
    register i16 c;
    register i16 xr;

    xr = x;
    c = color;
    if (0)
        xr = 0;
    font_set_colors(-1, c, c, c);
    return font_write(&scr_work, xr, y, menu_font, s, 0);
}
/* Writes s in the menu font in `color`, ending at x; returns the x it starts at. */
i16 write_right(char *s, i16 x, i16 y, i16 color)
{
    font_set_colors(-1, color, color, color);
    x -= font_string_width(0, menu_font, s);
    font_write(&scr_work, x, y, menu_font, s, 0);
    return x;
}
/* Writes s in `color`, centred in the w pixels from x (never left of 0). */
void write_centered(char *s, i16 x, i16 w, i16 y, i16 color)
{
    i16 left;

    left = (w >> 1) - (string_width(s) >> 1) + x;
    left = max(left, 0);
    write_in_color(s, left, y, color);
}
/* Writes s at (x, y) in the interface font, in the basic colour with the enhance colour as
 * its second glyph colour. */
void write_big_string(char *s, register i16 x, i16 y)
{
    font_set_colors(-1, basic_color, enhance_color, 0);
    font_write(&scr_work, x, y, font_inter, s, 0);
}
/* Writes s at (x, y) in the interface font, in `color` with `shadow` as the second glyph
 * colour. */
void write_big_in_color(char *s, register i16 x, i16 y, i16 color, i16 shadow)
{
    font_set_colors(-1, color, shadow, 0);
    font_write(&scr_work, x, y, font_inter, s, 0);
}
/* The same, ending at x; returns the x it starts at. */
i16 write_big_right(char *s, i16 x, i16 y, i16 color, i16 shadow)
{
    font_set_colors(-1, color, shadow, 0);
    x -= font_string_width(0, font_inter, s);
    font_write(&scr_work, x, y, font_inter, s, 0);
    return x;
}
/* The same, centred in the w pixels from x (never left of 0). */
void write_big_centered(char *s, i16 x, i16 w, i16 y, i16 color, i16 shadow)
{
    i16 left;

    left = (w >> 1) - (string_width_2(s) >> 1) + x;
    left = max(left, 0);
    write_big_in_color(s, left, y, color, shadow);
}
/* Appends the name of terrain t, with " Forest" after the forested ones (8 to 0x17). For a
 * negative t it appends string number region_size[13], which in the editor holds a continent
 * size, so the word would be arbitrary; nothing in the editor calls say_terrain. */
void say_terrain(char *buf, i16 t)
{
    if (t >= 0)
        say_string(buf, terrain[t].name);
    else
        say_string(buf, region_size[13]);
    if (t >= 8 && t < 0x18) {
        space_string(buf);
        say_string(buf, terrain_names[0]);
    }
}

