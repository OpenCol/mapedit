/* mapfile.c -- map files (.MP): the planes in memory, loading, saving, a blank map, the old forest codes. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "menu.h"
#include "buffer.h"
#include "error.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "fileio.h"
#include "loader.h"
#include "map.h"
#include "mapedit.h"
#include "mem.h"
#include "units.h"
/* MODULE map_9: MAPEDIT.EXE segment 09f9 (map_9.obj), file 0xb59a-0xba76, 8 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn set_plane_ports       0x0b59a
 * fn allocate_map_memory   0x0b5f2
 * fn initialize_map_memory 0x0b6b8
 * fn map_check             0x0b6bc
 * fn load_map_file         0x0b700
 * fn write_map_file        0x0b840
 * fn create_blank_map      0x0b94a
 * fn map_startup           0x0b9ce
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl25_70fa0).
 *
 * THE 1994 MAP-PLANE MODULE (segment para 283 of overlay 25): the Windows module 1038:ce56..d272
 * (allocate_map_memory, return_one_1038_cf4b, map_check, load_map_file, close_log_file,
 * create_blank_map, map_startup), whose twins are in. Same-file far calls go
 * through the two jump stubs at 0x7147c (map_check) and 0x71481 (allocate_map_memory).
 */

#include "records.h"

/* set_plane_ports -- 09f9:000a, 88 bytes */
/* DOS only: the four planes are plain far blocks, and each has a port record (height, width,
 * bits) for the buffer_get/buffer_put drawing code. */
void set_plane_ports(void)
{
    scr_map.w = scr_feature.w = scr_continent.w = scr_site.w = map_file.w;
    scr_map.h = scr_feature.h = scr_continent.h = scr_site.h = map_file.h;
    scr_map.bits = map;
    scr_feature.bits = feature;
    scr_continent.bits = continent;
    scr_site.bits = site;
}
/* allocate_map_memory -- 09f9:0062, 198 bytes */

/* Windows: MSAllocateHandle + MSHLock per plane, viewport 480x384. DOS: a too-big map
 * (map_high_memory_mode, set by map_check) gets 11956 bytes per plane, the viewport is 240x192, and on
 * success the plane ports are set up. */
i16 allocate_map_memory(i16 big)
{
    i16 err = 1;

    if (map_high_memory_mode)
        map_memory = 0x2eb4L;
    else
        map_memory = map_file.w * map_file.h;
    if (big != 0)
        map_memory = 12000L;
    map_pixel_size_x = 0xf0;
    map_pixel_size_y = 0xc0;
    map = mem_get(map_memory);
    if (map == 0)
        goto done;
    feature = mem_get(map_memory);
    if (feature == 0)
        goto done;
    continent = mem_get(map_memory);
    if (continent == 0)
        goto done;
    site = mem_get(map_memory);
    if (site == 0)
        goto done;
    set_plane_ports();
    err = 0;
done:
    return err;
}
/* initialize_map_memory -- 09f9:0128, 4 bytes */
i16 initialize_map_memory(void)
{
    return 1;
}
/* map_check -- 09f9:012c, 68 bytes */

/* Windows: returns 0. DOS: a map of more than 12000 cells sets map_high_memory_mode and error 9999. */
i16 map_check(void)
{
    i16 r = 1;

    if (map_size <= 12000L)
        map_high_memory_mode = 0;
    else
        map_high_memory_mode = 1;
    if (map_high_memory_mode) {
        map_error = 9999;
        goto out;
    }
    r = 0;
out:
    return r;
}
/* load_map_file -- 09f9:0170, 320 bytes */

i16 load_map_file(void)
{
    i16 err = 1;
    i16 ver;
    MSC_FILE *fh;

    fileio_new_ext(map_name, map_name, map_ext);
    fh = env_open(map_name, "rb");
    if (fh == 0) {
        map_error = 1;
        goto out;
    }
    if (msc_fread(&map_file, 4, 1, fh) == 0) {
        map_error = 2;
        goto out;
    }
    if (msc_fread(&ver, 2, 1, fh) == 0) {
        map_error = 2;
        goto out;
    }
    if ((ver > 4 || ver < 4) && map_file_version >= 0) {
        map_error = 3;
        goto out;
    }
    map_file_version = ver;
    map_size = (i32)map_file.w * map_file.h;
    if (map_check())
        goto out;
    if (map_high_memory_mode == 0) {
        if (fileio_fread_f(map, map_size, 1L, fh) == 0) {
            map_error = 4;
            goto out;
        }
        if (fileio_fread_f(feature, map_size, 1L, fh) == 0) {
            map_error = 5;
            goto out;
        }
        if (fileio_fread_f(continent, map_size, 1L, fh) == 0) {
            map_error = 6;
            goto out;
        }
    }
    map_error = err = 0;
    set_plane_ports();
out:
    if (fh != 0)
        msc_fclose(fh);
    return err;
}
/* write_map_file -- 09f9:02b0, 266 bytes */
/* Windows close_log_file is this function gutted: DOS writes the map file. */
i16 write_map_file(void)
{
    i16 err = 1;
    i16 ver;
    MSC_FILE *fh;

    fileio_new_ext(map_name, map_name, map_ext);
    fh = env_open(map_name, "wb");
    if (fh == 0) {
        map_error = 1;
        goto out;
    }
    if (msc_fwrite(&map_file, 4, 1, fh) == 0) {
        map_error = 2;
        goto out;
    }
    ver = map_file_version;
    if (msc_fwrite(&ver, 2, 1, fh) == 0) {
        map_error = 2;
        goto out;
    }
    map_size = (i32)map_file.w * map_file.h;
    if (map_high_memory_mode == 0) {
        if (fileio_fwrite_f(map, map_size, 1L, fh) == 0) {
            map_error = 4;
            goto out;
        }
        if (fileio_fwrite_f(feature, map_size, 1L, fh) == 0) {
            map_error = 5;
            goto out;
        }
        if (fileio_fwrite_f(continent, map_size, 1L, fh) == 0) {
            map_error = 5;
            goto out;
        }
    }
    map_error = err = 0;
out:
    if (fh != 0)
        msc_fclose(fh);
    return err;
}
/* create_blank_map -- 09f9:03ba, 132 bytes */

i16 create_blank_map(i16 w, i16 h)
{
    i16 err = 1;
    i16 wd;
    i16 ht;
    i16 n;

    map_file.w = w;
    map_file.h = h;
    map_size = (i32)w * h;
    if (map_check() == 0) {
        if (map_high_memory_mode == 0) {
            msc__fmemset(map, 0x19, (u16)map_size);
            msc__fmemset(feature, 0, (u16)map_size);
            msc__fmemset(continent, 0, (u16)map_size);
        }
        map_error = err = 0;
        map_file_version = 4;
    }
    return err;
}
/* map_startup -- 09f9:043e, 168 bytes */

i16 map_startup(i16 big)
{
    i16 err = 1;
    MSC_FILE *fh = 0;

    if (map_force == 0) {
        fileio_new_ext(map_name, map_name, map_ext);
        fh = env_open(map_name, "rb");
        map_file.w = 120;
        map_file.h = 75;
        map_size = (i32)map_file.w * map_file.h;
        if (fh != 0) {
            if (msc_fread(&map_file, 4, 1, fh) != 0)
                map_size = (i32)map_file.w * map_file.h;
        }
    }
    if (map_check() == 0) {
        if (allocate_map_memory(big) != 0)
            map_error = 0x13;
        else
            err = 0;
    }
    if (fh != 0)
        msc_fclose(fh);
    return err;
}

/* Old map files: a hills/mountain bit (0x20) on a forest square keeps only its base terrain
 * (bits 0xe0 and the low 3), and a plain forest (0x10-0x17) becomes its unforested type.
 * The `type -= 0x10; type += 8;` pair is what gives the shipped `add word [type],-8`: a
 * single -8 in any spelling comes out as `sub ..,8`. */
void forest_fix(void)
{
    i16 type;
    i16 byte;
    i16 x;
    i16 y;

    for (x = 0; x < map_file.w; x++) {
        for (y = 0; y < map_file.h; y++) {
            byte = buffer_get(x, y, scr_map);
            type = (u8)(byte & 0x1f);
            if (type >= 0x18)
                continue;
            if (byte & 0x20) {
                byte = byte & 0xe0;
                type = type & 7;
                buffer_put(x, y, byte | type, scr_map);
                continue;
            }
            if (type < 0x10 || type >= 0x18)
                continue;
            type = buffer_get(x, y, scr_map);
            type -= 0x10;
            type += 8;
            buffer_put(x, y, type, scr_map);
        }
    }
}
