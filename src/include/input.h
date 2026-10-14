/* input.h -- the editor's main loop and its input: keys, and mouse strokes by screen area.
 *
 * Defined in editor/input.c. turn_control_loop runs the editor once a map is loaded;
 * human_interface_loop is its event loop, which hands each key to the menu bar and then to
 * the editor's keys (parse_main_keys, parse_viewing_keys), and each mouse stroke to the menu
 * bar and then to the screen area it began in: the map view (parse_area_map), the minimap
 * (parse_area_mini) or the info window. A menu choice runs through execute_menu_event
 * (mapedit.h).
 */
#ifndef MAPEDIT_INPUT_H
#define MAPEDIT_INPUT_H

#include "msc.h"
extern i32 game_clock;          /* the 60 Hz clock, read once per loop pass */
extern i16 key_parsed;          /* some handler took the last key */
extern i16 mykey;               /* the key being handled (keys_get's code) */
extern i16 mouse_stroke_type;   /* the screen area the current stroke began in */
extern i16 mouse_current_area;  /* the screen area the mouse is in: 1 map, 2 minimap, 3 info */
extern i32 cursor_clock;        /* when the map cursor next blinks */
i16 event_wait(void);
i16 shift_key(void);
i16 mouse_area(void);
i16 parse_main_keys(void);
i16 parse_viewing_keys(void);
i16 parse_area_map(void);
void parse_area_mini(void);
i16 parse_mouse_stroke(void);
void human_interface_loop(void);
void human_turn(void);
void turn_control_loop(void);

#endif
