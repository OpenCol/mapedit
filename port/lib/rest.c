/* rest.c -- the last of the functions whose bodies were inline assembly: a string's width,
 * colour distance, sprite colour translation, palette cycling, the gradual map reveal and the
 * fatal-error report. Each as its original; tests/oracle checks the pure ones.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "msc.h"

/* font_string_width -- 0d6c:0002: the glyph widths of s, plus spacing after each glyph that
   has a width and is not the last character. (The original sign-extends the character, so a
   code from 80h up reads before the width table; the port reads it as unsigned.) */
i16 font_string_width(i16 spacing, u8 *font, char *s)
{
    i16 total = 0;
    const u8 *p = (const u8 *)s;
    if (!*p)
        return 0;
    do {
        u8 w = font[2 + (u8)(*p - 1)];
        i16 c = w;
        p++;
        if (c > 0 && *p != 0)
            c = (i16)(c + spacing);
        total = (i16)(total + c);
    } while (*p);
    return total;
}

/* pal_get_hash -- 1074:0288: the squared distance of two colours, each difference a signed
   byte (it wraps), squared signed, summed in 16 bits */
i16 pal_get_hash(u8 *b, u8 *a)
{
    i16 sum = 0, i;
    for (i = 0; i < 3; i++) {
        i8 d = (i8)(u8)(a[i] - b[i]);
        sum = (i16)(sum + (i16)d * d);
    }
    return sum;
}

/* sprite_color_translate -- 1069:0006: rewrite every frame's colours in place through map: a
   colour c becomes map[5 + 6c] (the palette list's index field). A frame's rows end with a
   row opening FCh; FFh is an empty row, FDh a row of runs, anything else a plain row whose
   FEh runs keep their count. Transparent FDh pixels stay. A frame with no pixels ends it. */
#include "series.h"

void sprite_color_translate(void *sheet, char *map)
{
    SpriteSeriesView *ss = sheet;
    const u8 *tab = (const u8 *)map + 5;
    i16 i;
    for (i = 0; i < ss->count; i++) {
        u8 *s = ss->icon[i].data;
        if (s == 0)
            break;
        for (;;) {
            u8 mode = *s++;
            if (mode == 0xfc)
                break;
            if (mode == 0xff)
                continue;
            if (mode == 0xfd) {
                for (;;) {
                    if (*s++ == 0xff)
                        break;
                    if (*s != 0xfd)
                        *s = tab[*s * 6];
                    s++;
                }
            } else {
                for (;;) {
                    if (*s == 0xff) {
                        s++;
                        break;
                    }
                    if (*s == 0xfe)
                        s += 2;
                    if (*s != 0xfd)
                        *s = tab[*s * 6];
                    s++;
                }
            }
        }
    }
}

/* cycle_colors -- 0f1a:00a6: every due range of the cycling palette turns by one colour (its
   last moves to its front), and the cycled colours go to the DAC */
typedef struct { u8 count, pos, start, speed; } CycleRange;
typedef struct { i16 n; CycleRange r[8]; } ColorCycle;
extern ColorCycle cycle_list;
extern i32 cycle_timing[8];
extern i16 total_cycle_colors, cycling_threshold, cycling_active, cycling_delay;
extern i16 mcga_palette_update;
extern u8 cycling_palette[0x300];
i32 timer_read(void);
void mcga_setpal_range(i16 first, i16 count, char *pal);

void cycle_colors(void)
{
    i32 now;
    i16 i, changed = 0;
    if (cycling_active == 0)
        return;
    if (++cycling_delay < cycling_threshold)
        return;
    if (mcga_palette_update != 0)
        return;
    now = timer_read();
    for (i = 0; i < cycle_list.n; i++) {
        CycleRange *r = &cycle_list.r[i];
        if (r->speed + cycle_timing[i] <= now) {
            i16 cnt = r->count, start = r->start;
            u16 pos = r->pos;
            cycle_timing[i] = now;
            changed = -1;
            if (cnt > 1) {
                u8 *p = cycling_palette + start * 3, last[3];
                memcpy(last, p + (cnt - 1) * 3, 3);
                memmove(p + 3, p, (size_t)(cnt - 1) * 3);
                memcpy(p, last, 3);
                pos++;
                if (pos >= (u16)cnt)
                    pos = 0;
            }
            r->pos = (u8)pos;
        }
    }
    if (changed)
        mcga_setpal_range(cycle_list.r[0].start, total_cycle_colors, (char *)cycling_palette);
    cycling_delay = 0;
}

/* show_map_gradually -- 087b:0160: copy the map view to the screen a square at a time, in the
   order of a 16-bit Galois LFSR (taps B400h) over 65,536 steps; a small view (180 squares or
   fewer) waits a 600 Hz tick between squares */
extern i16 map_view_size_x, map_view_size_y, map_tile_inset_x, map_tile_inset_y;
extern i16 map_tile_size_x, map_tile_size_y;
extern i32 timer_600_low;
i32 timer_read_600(void);
void mouse_video_update(i16 sx, i16 sy, i16 dx, i16 dy, i16 w, i16 h);

void show_map_gradually(void)
{
    u16 lfsr = 1, total, pos, count = 0;
    i32 oldt = 0;
    total = (u16)(map_view_size_y * map_view_size_x);
    timer_600_low = 0;
    for (;;) {
        lfsr = (u16)((lfsr & 1) ? (lfsr >> 1) ^ 0xb400 : lfsr >> 1);
        pos = (u16)(lfsr - 1);
        if (pos < total) {
            i16 col = (i16)(pos % (u16)map_view_size_x), row = (i16)(pos / (u16)map_view_size_x);
            i16 sx = (i16)((col + map_tile_inset_x) * map_tile_size_x);
            i16 sy = (i16)((row + map_tile_inset_y) * map_tile_size_y + 8);
            mouse_video_update(sx, sy, sx, sy, map_tile_size_x, map_tile_size_y);
            if (total <= 0xb4) {
                if (oldt != 0)
                    while (timer_read_600() - oldt < 1)
                        ;
                oldt = timer_read_600();
            }
        }
        if (++count == 0)
            break;
    }
}

/* error_shutdown_report -- 0ed0:0156: shut everything down, back to text mode, print the
   report (and the warning file), call the service routines, exit with 3. The original's one
   piece of assembly set text mode through the BIOS; the port closes its window. */
i16 sound_initialized(void);
void sound_queue(i16 on);
void sound_system_shutdown(void);
void timer_remove(void);
void keys_remove(void);
void himem_shutdown(void);
i16 mouse_init(i16 on, i16 mode);
void screen_dominant_mode(i16 mode);
void echo(i16 kind, char *text);
void error_dump_file(char *name);
void host_quit(void);
extern char mem_last_alloc_failed;
extern i32 mem_last_alloc_size, mem_last_alloc_avail;
extern char error_string[];
extern void (*error_service_routine)(void), (*error_service_routine_2)(void);

void error_shutdown_report(char *errtext, char *modtext, char *numtext, char *extra, i32 memfree, i16 code)
{
    char line[80];
    if (sound_initialized()) {
        sound_queue(0);
        sound_system_shutdown();
    }
    timer_remove();
    keys_remove();
    himem_shutdown();
    mouse_init(0, 3);
    screen_dominant_mode(3);
    host_quit();                        /* PORT: was INT 10h, AX=3: text mode */
    if (code != -20) {
        snprintf(line, sizeof line, "Error \"%s\" in module \"%s\" data: %s %s", errtext, modtext, numtext, extra);
        echo(-1, line);
        if (strlen(error_string))
            echo(-1, error_string);
        if (mem_last_alloc_failed) {
            snprintf(line, sizeof line, "Tried to allocate %ld bytes when only %ld bytes were free.",
                     (long)mem_last_alloc_size, (long)mem_last_alloc_avail);
            echo(-1, line);
        }
        echo(-1, " ");
        snprintf(line, sizeof line, "Most recent memory check: %ld", (long)memfree);
        echo(-1, line);
        echo(-1, "Most recent DOS error:    0");
        echo(-1, "Stack use indicator:      0");
        echo(-1, " ");
        error_dump_file("*warn0.dat");
    } else
        error_dump_file("*warn6.dat");
    if (error_service_routine_2)
        (*error_service_routine_2)();
    if (error_service_routine)
        (*error_service_routine)();
    exit(3);
}
