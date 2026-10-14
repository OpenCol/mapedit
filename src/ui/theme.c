/* theme.c -- the editor's colours and fonts, as the popups, menus and text use them.
 *
 * The colours are palette indices. They start at the defaults below; theme_load replaces nine
 * of them from the COLORS section of NAMES.TXT and hands them to the popup library
 * (popups_normal). The editor's main copies the same values into the menu bar and its menus.
 */
#include "msc.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as the 1994 compiler did */
#include "text.h"
#include "assets.h"
#include "error.h"
#include "font.h"
#include "pack.h"
#include "popup.h"
#include "theme.h"

u8 basic_color = 0x44;               /* ordinary text (glyph pixels of value 1) */

u8 hilite_color = 0x95;              /* hot keys and {highlighted} spans */

u8 grey_color = 0x08;                /* greyed (disabled) items */

u8 enhance_color = 0x80;             /* glyph pixels of value 2 */

u8 shadow_color = 0x2f;              /* glyph pixels of value 3 */

u8 select_color = 0x8a;              /* the bar behind the selected item */

u8 notice_color = 0x95;              /* not in COLORS and not read by the editor */

u8 border0 = 0x86;                   /* the popup frame's outer box */

u8 border1 = 0x80;                   /* its inner top and right lines */

u8 border2 = 0x8a;                   /* its inner left and bottom lines */

DosFont *menu_font = 0;                     /* the small font (FONTTINY) of menus and panels */

char label[50];                           /* unused */

/* Sets the popup library's colours, border, backdrop tile (scr_back) and palette flag from
 * the editor's colours above. */
void popups_normal(void)
{
    popup_enhance_color = enhance_color;
    popup_shadow_color = shadow_color;
    popup_font_main_color = basic_color;
    popup_font_hilite_color = hilite_color;
    popup_font_grey_color = grey_color;
    popup_select_color_0 = popup_select_color_1 = select_color;
    popup_border_color = border0;
    popup_border_color_up = border1;
    popup_border_color_down = border2;
    popup_tile = &scr_back;
    popup_pal = 1;
}

/* Reads the COLORS section of the text file load_data has open (NAMES.TXT): one line of nine
 * numbers, in the order basic, hilite, grey, enhance, shadow, select, border0, border1,
 * border2. Without the section the defaults stay. Either way the popups then take the
 * colours (popups_normal). */
void theme_load(void)
{
    if (text_open(0, "COLORS") == 0) {
        text_get();
        basic_color = text_item_number();
        hilite_color = text_item_number();
        grey_color = text_item_number();
        enhance_color = text_item_number();
        shadow_color = text_item_number();
        select_color = text_item_number();
        border0 = text_item_number();
        border1 = text_item_number();
        border2 = text_item_number();
    }
    popups_normal();
}
