/* input.h */
#ifndef MAPEDIT_INPUT_H
#define MAPEDIT_INPUT_H

#include "msc.h"
extern i32 game_clock;
extern i16 key_parsed;
extern i16 mykey;
extern i16 mouse_stroke_type;
extern i16 mouse_current_area;
extern i32 cursor_clock;
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
