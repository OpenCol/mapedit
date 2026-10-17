/* png.h -- writing a picture as a PNG file (png.c), for Export Map as PNG. */
#ifndef MAPEDIT_PNG_H
#define MAPEDIT_PNG_H

#include <stdint.h>

/* w x h palette indices, row-major, and 256 colours of 6-bit DAC values (r, g, b each 0-63),
 * to the host file path; 0 when written, -1 when not (nothing is left behind) */
int port_write_png(const char *path, int w, int h, const uint8_t *pixels, const uint8_t *dac);

/* the same as file `name` in folder dir (NULL or "": the current folder), else in folder
 * fallback (NULL: none); 0 with the file's path in where (cap bytes), or -1 */
int port_export_png(const char *name, int w, int h, const uint8_t *pixels, const uint8_t *dac,
                    const char *dir, const char *fallback, char *where, int cap);

#endif
