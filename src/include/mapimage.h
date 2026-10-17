/* mapimage.h -- Export Map as PNG (port: options.h, export_png).
 *
 * Defined in ui/mapimage.c. export_map_png draws the whole map at zoom 0 into a picture and
 * writes it as NAME.png, NAME being the map's (port/png.c): in editor_options.export_dir, or
 * in export_fallback when that cannot be written. It leaves the view as it found it, but
 * scr_orig holds the last part drawn: the caller redraws the screen. Returns 0 with the
 * file's path in where (cap bytes), or -1.
 */
#ifndef MAPEDIT_MAPIMAGE_H
#define MAPEDIT_MAPIMAGE_H

#include "msc.h"

i16 export_map_png(char *where, i16 cap);

#endif
