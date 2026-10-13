/* Imported from mapedit-decomp src/editor/write by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE write: MAPEDIT.EXE segment 08ad (write.obj), file 0xa0d0-0xa594, 30 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn space_string       0x0a0d0
 * fn spacer             0x0a0e0
 * fn comma_string       0x0a102
 * fn colon_string       0x0a112
 * fn period_string      0x0a122
 * fn percent_string     0x0a132
 * fn open_string        0x0a142
 * fn close_string       0x0a152
 * fn brace_on           0x0a162
 * fn brace_off          0x0a172
 * fn plus_string        0x0a182
 * fn minus_string       0x0a192
 * fn times_string       0x0a1a2
 * fn say_string         0x0a1b2
 * fn say_braced         0x0a1cc
 * fn say_number         0x0a1fe
 * fn say_binary         0x0a226
 * fn say_long           0x0a28e
 * fn say_bucks          0x0a2b8
 * fn string_width       0x0a2e6
 * fn string_width_2     0x0a302
 * fn write_string       0x0a31e
 * fn write_in_color     0x0a358
 * fn write_right        0x0a392
 * fn write_centered     0x0a3e8
 * fn write_big_string   0x0a42c
 * fn write_big_in_color 0x0a46a
 * fn write_big_right    0x0a4a2
 * fn write_big_centered 0x0a500
 * fn say_terrain        0x0a548
 *
 * TEXT BUILDING AND DRAWING (root code, frame 004b). Engine code with Windows twins for the
 * say_* family (`say_number` and others). The string tails each append one of
 * this module's literals (DS:0x50-0x6f, in function order). DOS buffers are near where the
 * fixed tails write and far where the say_* functions write.
 *
 * The flags change by function: "el" for the string builders, write_right and write_big_right;
 * "el" off with a `register int x` parameter for write_string, write_big_string and
 * write_big_in_color.
 * write_centered and write_big_centered compute the left edge into a new local and clamp it with its own
 * statement (`left = max(left, 0);`). Reassigning the parameter x keeps x in si from entry;
 * the new local lets si hold s's segment until the width call, as shipped.
 *
 *  No pads needed after it (0..16 all keep the other 29).
 * The old note (register PARAMETERS, 6 bytes) follows. Shipped has x in di and color
 *     in si; ours is the reverse, and that naming is all that is left. So the shipped pairing (di to the earlier
 *     parameter) is not reachable by `register` as such. Also ruled out: pads 0..16 before
 *     the function, K&R declaration order with the register parameter written first, a
 *     register local for either value with the assignments in both orders, and mixing a
 *     register parameter with a register local (that pair gets si/di the right way round but
 *     homes si first, where shipped homes di first).
 * more axes closed. (a) K&R: the register assignment follows the
 *     IDENTIFIER LIST, not the declaration block, so writing `register int color;` above
 *     `register int x;` in a K&R parameter list changes nothing -- six orderings, all 6 bytes.
 *     (b) With the global register allocator on ("e", "el", "elg", "ael", "cel", "elt",
 *     "aelg", with or without `register` on either parameter) MSC ranks by use count: color,
 *     read three times, takes si and x is read straight from [bp+0xa], which is 54 bytes, 4
 *     short. Nothing makes it enregister a parameter with one use -- not `register`, not a
 *     register local copy assigned at the top. (c) Two register LOCALS do not both get a
 *     register: declaring color then x and assigning them in either order gives 54-56 bytes.
 *     So si goes to the earlier register parameter under every flag set, and the shipped
 *     pairing stays out of reach.
 *     font_write(&scr_work, color ? x : x, y, menu_font, s, 0) -- an identical-arm ?:
 *     compiles to nothing but counts (statement-level dead uses, `x;`, `x = x;`,
 *     `(void)x;`, `if (x == x)`, and ?: in any other argument slot are dropped before the
 *     count or add code) -- gives x two uses against color's three: x -> di, color -> si,
 *     homed in the shipped order, 58 bytes, 4 differ: the ?: makes x a computed argument,
 *     so `mov ax,di` is emitted before `lea bx,[scr_work]` where shipped has the lea
 *     first. A ?: on the pointer argument instead folds to `mov bx,imm` (12 bytes), and no
 *     pragma letter (w/t/s/a/c/p/n/z on or off with e) changes the order. Not adopted: the
 *     construct is not a plausible source, and 4 bytes is not a match. Whatever gave the
 *     original x its second use is still unknown.
 * the 4-byte residue under "e" is not the ?: itself: ANY way of giving x a
 *     second use (`if ((unsigned)x >= 0) font_set_colors(..)`, `return (unsigned)x < 0 ? 0 :
 *     font_write(..)`, an early `return 0`) gives the same 4 bytes. Pads 0..16
 *     under "e" (34 variants), unsigned x/y on the prototype or the definition, casts, and
 *     "e" crossed with l/g/lg/a/w/z/p/t do not move it. Plain: `register` on s, y, an unused
 *     register local, and xr/c register-local copies (si/di right only with color a register
 *     PARAMETER and x a register LOCAL, but its load sinks to the call) do not either.
 */

// MSC: #pragma optimize("el", on)
#define max(a, b) ((a) >= (b) ? (a) : (b))
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;


extern char *strings(i16 word);                    /* 0000:0062 */
extern i16 font_string_width(i16 z, void *font, char *s);
extern void *menu_font;                               /* DS:0x0080 */
extern void *font_inter;                           /* DS:0x3a96 */
extern void font_set_colors(i16 a, i16 b, i16 c, i16 d);
extern i16 font_write(DosPort *port, i16 x, i16 y, void *font, char *s, i16 flags);
extern DosPort scr_work;                          /* DS:0x3af4 */
extern u8 basic_color;                            /* DS:0x0092: the text colour */
extern u8 enhance_color;                            /* DS:0x0095: the big text's shadow */
typedef struct { i16 word; char rest[14]; } TerrainName;
extern TerrainName terrain[];                  /* DS:0x4ee6 */
extern i16 region_size[16];                            /* DS:0x5270 (map.obj); DS:0x528a is region_size[13], the word for t < 0 */
extern i16 terrain_names[];                            /* DS:0x4b5a: five words, mapedit.obj */
/* space_string -- 08ad:0000, 16 bytes */
void space_string(char *buf)
{
    msc_strcat(buf, " ");
}
/* spacer -- 08ad:0010, 34 bytes */
void spacer(char *buf, i16 n)
{
    i16 i;

    for (i = 0; i < n; i++)
        space_string(buf);
}
/* comma_string -- 08ad:0032, 16 bytes */
void comma_string(char *buf)
{
    msc_strcat(buf, ", ");
}
/* colon_string -- 08ad:0042, 16 bytes */
void colon_string(char *buf)
{
    msc_strcat(buf, ": ");
}
/* period_string -- 08ad:0052, 16 bytes */
void period_string(char *buf)
{
    msc_strcat(buf, ".  ");
}
/* percent_string -- 08ad:0062, 16 bytes */
void percent_string(char *buf)
{
    msc_strcat(buf, "%");
}
/* open_string -- 08ad:0072, 16 bytes */
void open_string(char *buf)
{
    msc_strcat(buf, "(");
}
/* close_string -- 08ad:0082, 16 bytes */
void close_string(char *buf)
{
    msc_strcat(buf, ")");
}
/* brace_on -- 08ad:0092, 16 bytes */
void brace_on(char *buf)
{
    msc_strcat(buf, "{");
}
/* brace_off -- 08ad:00a2, 16 bytes */
void brace_off(char *buf)
{
    msc_strcat(buf, "}");
}
/* plus_string -- 08ad:00b2, 16 bytes */
void plus_string(char *buf)
{
    msc_strcat(buf, "+");
}
/* minus_string -- 08ad:00c2, 16 bytes */
void minus_string(char *buf)
{
    msc_strcat(buf, "-");
}
/* times_string -- 08ad:00d2, 16 bytes */
void times_string(char *buf)
{
    msc_strcat(buf, "x");
}
/* say_string -- 08ad:00e2, 26 bytes */
void say_string(char *buf, i16 word)
{
    msc__fstrcat(buf, strings(word));
}
/* say_braced -- 08ad:00fc, 50 bytes */
void say_braced(char *buf, i16 word)
{
    brace_on(buf);
    msc__fstrcat(buf, strings(word));
    brace_off(buf);
}
/* say_number -- 08ad:012e, 40 bytes */
void say_number(char *s, i16 number)
{
    char buf[0x14];

    msc_itoa(number, buf, 10);
    msc__fstrcat(s, buf);
}
/* say_binary -- 08ad:0156, 104 bytes */
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
/* say_long -- 08ad:01be, 42 bytes */
void say_long(char *s, i32 number)
{
    char buf[0x14];

    msc_ltoa(number, buf, 10);
    msc__fstrcat(s, buf);
}
/* say_bucks -- 08ad:01e8, 46 bytes */
void say_bucks(char *s, i32 number)
{
    say_long(s, number);
    msc__fstrcat(s, "$");
}
/* string_width -- 08ad:0216, 28 bytes */
i16 string_width(char *s)
{
    return font_string_width(0, menu_font, s) - 1;
}
/* string_width_2 -- 08ad:0232, 28 bytes */
i16 string_width_2(char *s)
{
    return font_string_width(0, font_inter, s) - 1;
}
/* write_string -- 08ad:024e, 58 bytes */
// MSC: #pragma optimize("el", off)   /* the drawing helpers: register locals, no global optimiser */

i16 write_string(char *s, register i16 x, i16 y)
{
    font_set_colors(-1, basic_color, 0, 0);
    return font_write(&scr_work, x, y, menu_font, s, 0);
}
/* write_in_color -- 08ad:0288, 58 bytes */
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
/* write_right -- 08ad:02c2, 86 bytes */
// MSC: #pragma optimize("el", on)
i16 write_right(char *s, i16 x, i16 y, i16 color)
{
    font_set_colors(-1, color, color, color);
    x -= font_string_width(0, menu_font, s);
    font_write(&scr_work, x, y, menu_font, s, 0);
    return x;
}
/* write_centered -- 08ad:0318, 68 bytes */
// MSC: #pragma optimize("el", on)
void write_centered(char *s, i16 x, i16 w, i16 y, i16 color)
{
    i16 left;

    left = (w >> 1) - (string_width(s) >> 1) + x;
    left = max(left, 0);
    write_in_color(s, left, y, color);
}
/* write_big_string -- 08ad:035c, 62 bytes */
// MSC: #pragma optimize("el", off)
void write_big_string(char *s, register i16 x, i16 y)
{
    font_set_colors(-1, basic_color, enhance_color, 0);
    font_write(&scr_work, x, y, font_inter, s, 0);
}
/* write_big_in_color -- 08ad:039a, 56 bytes */
void write_big_in_color(char *s, register i16 x, i16 y, i16 color, i16 shadow)
{
    font_set_colors(-1, color, shadow, 0);
    font_write(&scr_work, x, y, font_inter, s, 0);
}
/* write_big_right -- 08ad:03d2, 94 bytes */
// MSC: #pragma optimize("el", on)
i16 write_big_right(char *s, i16 x, i16 y, i16 color, i16 shadow)
{
    font_set_colors(-1, color, shadow, 0);
    x -= font_string_width(0, font_inter, s);
    font_write(&scr_work, x, y, font_inter, s, 0);
    return x;
}
/* write_big_centered -- 08ad:0430, 72 bytes */
void write_big_centered(char *s, i16 x, i16 w, i16 y, i16 color, i16 shadow)
{
    i16 left;

    left = (w >> 1) - (string_width_2(s) >> 1) + x;
    left = max(left, 0);
    write_big_in_color(s, left, y, color, shadow);
}
/* say_terrain -- 08ad:0478, 76 bytes */
void say_terrain(char *buf, i16 t)
{
    if (t >= 0)
        say_string(buf, terrain[t].word);
    else
        say_string(buf, region_size[13]);
    if (t >= 8 && t < 0x18) {
        space_string(buf);
        say_string(buf, terrain_names[0]);
    }
}

