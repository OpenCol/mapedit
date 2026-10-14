/* mapfile.c -- .MP map files: the planes' memory, loading, saving, a blank map, forest codes.
 *
 * A .MP file is a six-byte header and three planes, all little-endian:
 *
 *     i16 width, i16 height     map_file, in squares
 *     i16 version               4; the editor refuses any other
 *     width * height bytes      the terrain plane (map)
 *     width * height bytes      the feature plane (feature)
 *     width * height bytes      the continent plane (continent)
 *
 * The sight plane (site) is not in the file. map_startup allocates the four planes once, at
 * startup, 12000 bytes each; every later map is loaded into (or created in) those same
 * blocks, which map_check makes safe by refusing a map of more than 12000 squares. The file
 * is map_name with its extension replaced by .MP, in the game folder.
 *
 * Loading, saving and creating return 0 on success and 1 on failure, the reason in map_error:
 * 1 the file does not open, 2 the header is short, 3 the version is not 4, 4-6 a plane is
 * short (terrain, features, continents), 9999 the map is too big, 0x13 out of memory.
 */
#include "msc.h"
#pragma pack(2)   /* 2-byte record packing, as in 1994: the file formats depend on it */
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

#include "records.h"

/* Describes each plane as a map_file.w x map_file.h port (scr_map, scr_feature,
 * scr_continent, scr_site), so the buffer code can read and write it by (x, y). */
void set_plane_ports(void)
{
    scr_map.w = scr_feature.w = scr_continent.w = scr_site.w = map_file.w;
    scr_map.h = scr_feature.h = scr_continent.h = scr_site.h = map_file.h;
    scr_map.bits = map;
    scr_feature.bits = feature;
    scr_continent.bits = continent;
    scr_site.bits = site;
}

/* Allocates the four planes, map_memory bytes each: 12000 when big is nonzero (the editor
 * always passes 1), else the map's size (11956 under map_high_memory_mode). Sets the view to
 * 240 x 192 pixels and, on success, the plane ports. Returns 0, or 1 when a block cannot be
 * had; the blocks already allocated are then kept. */
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

/* Does nothing; returns 1. */
i16 initialize_map_memory(void)
{
    return 1;
}

/* Can the planes hold a map of map_size squares? Returns 0 if so (12000 or fewer); else sets
 * map_high_memory_mode and map_error 9999 and returns 1. */
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

/* Loads map_name (as .MP) into the planes: sets map_file, map_file_version, map_size and the
 * plane ports. The version test also passes any version while map_file_version is negative,
 * which it never is. A failed load is not undone: map_file already holds the new header (and
 * a short plane may be half read), while the planes still hold the old map. */
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

/* Saves the map to map_name (as .MP), with the version it was loaded or created with. A
 * failure to write the continent plane reports error 5, as the feature plane does. */
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

/* Makes a new w x h map of ocean (terrain 0x19, no features, continent bytes 0, so region 0
 * and owner 0 rather than nobody), version 4. A map too big for the planes returns 1, with
 * map_file and map_size already changed. The plane ports are not updated: they keep the
 * previous map's width and height. */
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

/* Allocates the planes at startup (allocate_map_memory(big)). Unless map_force is set, it
 * first reads the width and height from map_name's header (120 x 75 when there is no file),
 * so a map too big for memory is refused before anything is allocated; the editor always
 * sets map_force and keeps the map_file it set itself. Returns 0, or 1 with map_error 0x13
 * when memory runs out (or 9999 from map_check). */
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

/* Normalises the forests of a just-loaded map: a hills or mountains square (MAP0_HILLY) whose
 * low bits name a forest (8-23) keeps only the open terrain under it (0-7), and a forest of
 * the second band (16-23) becomes the same forest in the first (8-15). The top three bits,
 * rivers included, are kept. */
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
