/* theme.c -- the editor's colours and fonts, as the popups, menus and text use them. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "text.h"
#include "assets.h"
#include "error.h"
#include "font.h"
#include "pack.h"
#include "popup.h"
#include "theme.h"

u8 basic_color = 0x44;               /* DS:0x0092 */

u8 hilite_color = 0x95;              /* DS:0x0093 */

u8 grey_color = 0x08;                /* DS:0x0094 */

u8 enhance_color = 0x80;             /* DS:0x0095 */

u8 shadow_color = 0x2f;              /* DS:0x0096 */

u8 select_color = 0x8a;              /* DS:0x0097 */

u8 notice_color = 0x95;              /* DS:0x0098 */

u8 border0 = 0x86;                   /* DS:0x0099 */

u8 border1 = 0x80;                   /* DS:0x009a */

u8 border2 = 0x8a;                   /* DS:0x009b */

DosFont *menu_font = 0;                     /* DS:0x0080 */

char label[50];                           /* DS:0x523a: 50 bytes, unused */

/* The popup colours for the editor's screens, from the editor's colour bytes. */
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

/* theme_load -- the COLORS section of the text file load_data reads (NAMES.TXT, still open),
   then the popups' colours from them. Split out of load_data, which read on into it. */
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
