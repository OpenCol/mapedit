/* png.c -- writing a picture as a PNG file, for Export Map as PNG (ui/mapimage.c).
 *
 * An 8-bit palette image: the pixels are palette indices, the palette 256 colours of 6-bit
 * VGA DAC values (0-63), as the editor keeps them. The image data is stored, not compressed
 * (zlib's stored blocks), so the writer needs no library: a 56 x 70 map at 16 pixels a square
 * is about a megabyte.
 *
 * The file is written outside the game folder, by a host path: the DOS names of the run-time
 * (msc_runtime.c) do not reach it.
 */
#include <stdio.h>
#include <string.h>

#include <SDL3/SDL.h>

#include "png.h"

static uint32_t crc_table[256];

static uint32_t crc(uint32_t c, const uint8_t *p, size_t n)
{
    size_t i;
    if (crc_table[1] == 0) {
        uint32_t k, v;
        int j;
        for (k = 0; k < 256; k++) {
            v = k;
            for (j = 0; j < 8; j++)
                v = v & 1 ? 0xedb88320u ^ (v >> 1) : v >> 1;
            crc_table[k] = v;
        }
    }
    for (i = 0; i < n; i++)
        c = crc_table[(c ^ p[i]) & 0xff] ^ (c >> 8);
    return c;
}

static void be32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

/* one chunk: length, type, data, CRC of type and data */
static int chunk(SDL_IOStream *io, const char *type, const uint8_t *data, uint32_t n)
{
    uint8_t head[8], tail[4];
    uint32_t c;
    be32(head, n);
    memcpy(head + 4, type, 4);
    c = crc(0xffffffffu, head + 4, 4);
    c = crc(c, data, n) ^ 0xffffffffu;
    be32(tail, c);
    return SDL_WriteIO(io, head, 8) == 8 && (n == 0 || SDL_WriteIO(io, data, n) == n)
        && SDL_WriteIO(io, tail, 4) == 4;
}

int port_write_png(const char *path, int w, int h, const uint8_t *pixels, const uint8_t *dac)
{
    static const uint8_t sig[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n'};
    uint8_t ihdr[13], plte[768];
    size_t row = (size_t)w + 1, raw = row * (size_t)h, blocks = (raw + 65534) / 65535;
    size_t zlen = 2 + raw + 5 * blocks + 4, pos, done, i;
    uint8_t *z;
    uint32_t a = 1, b = 0;
    SDL_IOStream *io;
    int ok;

    if (w <= 0 || h <= 0)
        return -1;
    z = SDL_malloc(zlen);
    if (!z)
        return -1;
    /* zlib: header, stored blocks of the filtered rows (filter 0 each), Adler-32 */
    z[0] = 0x78;
    z[1] = 0x01;
    pos = 2;
    for (done = 0; done < raw;) {
        size_t n = raw - done > 65535 ? 65535 : raw - done;
        z[pos++] = done + n == raw;
        z[pos++] = (uint8_t)n;
        z[pos++] = (uint8_t)(n >> 8);
        z[pos++] = (uint8_t)~n;
        z[pos++] = (uint8_t)(~n >> 8);
        for (i = 0; i < n; i++, done++) {
            size_t x = done % row, y = done / row;
            uint8_t v = x == 0 ? 0 : pixels[y * (size_t)w + x - 1];
            z[pos++] = v;
            a = (a + v) % 65521;
            b = (b + a) % 65521;
        }
    }
    be32(z + pos, b << 16 | a);
    pos += 4;

    be32(ihdr, (uint32_t)w);
    be32(ihdr + 4, (uint32_t)h);
    ihdr[8] = 8;                    /* bits per pixel */
    ihdr[9] = 3;                    /* palette */
    ihdr[10] = ihdr[11] = ihdr[12] = 0;
    for (i = 0; i < 768; i++)
        plte[i] = (uint8_t)((dac[i] & 63) << 2 | (dac[i] & 63) >> 4);

    io = SDL_IOFromFile(path, "wb");
    ok = io && SDL_WriteIO(io, sig, 8) == 8 && chunk(io, "IHDR", ihdr, 13) && chunk(io, "PLTE", plte, 768)
        && chunk(io, "IDAT", z, (uint32_t)pos) && chunk(io, "IEND", NULL, 0);
    if (io && !SDL_CloseIO(io))
        ok = 0;
    SDL_free(z);
    if (io && !ok)
        SDL_RemovePath(path);
    return ok ? 0 : -1;
}

/* dir + "/" + name, or name alone for the current folder (dir NULL or "") */
static void join(char *out, size_t cap, const char *dir, const char *name)
{
    if (!dir || !*dir)
        snprintf(out, cap, "%s", name);
    else {
        size_t n = strlen(dir);
        snprintf(out, cap, "%s%s%s", dir, n && (dir[n - 1] == '/' || dir[n - 1] == '\\') ? "" : "/", name);
    }
}

int port_export_png(const char *name, int w, int h, const uint8_t *pixels, const uint8_t *dac,
                    const char *dir, const char *fallback, char *where, int cap)
{
    char path[1024];
    join(path, sizeof path, dir, name);
    if (port_write_png(path, w, h, pixels, dac) != 0) {
        if (!fallback)
            return -1;
        join(path, sizeof path, fallback, name);
        if (port_write_png(path, w, h, pixels, dac) != 0)
            return -1;
        dir = fallback;
    }
    if (!dir || !*dir) {
        char *cwd = SDL_GetCurrentDirectory();
        join(where, (size_t)cap, cwd, name);
        SDL_free(cwd);
    } else
        snprintf(where, (size_t)cap, "%s", path);
    return 0;
}
