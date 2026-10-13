/* msc_runtime.c -- the parts of Microsoft C 6.00A's run-time library the editor calls.
 *
 * The import renamed each of them msc_<name> (tools/import.py), so they never meet the
 * modern C library's. They behave as MSC 6's did where the editor can tell:
 *
 *   - Files are DOS files: a name such as "C:\MPS\COLONIZE\MAPEDIT.TXT" or "viceroy.pal" is
 *     looked up in the game directory (msc_set_root), each component matched without regard
 *     to case, as DOS's FAT names were. A name that does not exist yet is created in upper
 *     case.
 *   - Handles are small integers, as DOS gave them (0-4 are the standard devices), because
 *     the library keeps one in FILE._file and calls DOS on it.
 *   - stdio is MSC's own: the FILE record's buffer pointer, count and flags mean what they
 *     meant in 1994, since fileio_fread_f reads and moves them itself. Text mode ("t", the
 *     default) drops the CR of each CR-LF on input and ends the file at a Ctrl-Z, and adds a
 *     CR before each LF on output.
 *   - Integers are 16 bits wide where MSC's were: atoi wraps, itoa writes a 16-bit value.
 *
 * The host side is SDL3's (files, directories, path information), so the same code runs on
 * Windows, macOS and Linux. A directory is listed in name order, case ignored: DOS gave its
 * own order (the directory's), which no host file system keeps.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

#define MSC_RUNTIME_IMPL
#include "msc.h"

/* DOS's open() flags, as the source passes them */
#define DOS_WRONLY 0x0001
#define DOS_RDWR   0x0002
#define DOS_APPEND 0x0008
#define DOS_CREAT  0x0100
#define DOS_TRUNC  0x0200
#define DOS_EXCL   0x0400
#define DOS_BINARY 0x8000
#define MSC_EOF    (-1)

#define MSC_NFILE  20           /* MSC 6's _NFILE */
#define MSC_BUFSIZ 512          /* MSC 6's BUFSIZ */
#define NHANDLES   40

#define M_IOREAD  0x01
#define M_IOWRT   0x02
#define M_IONBF   0x04
#define M_IOMYBUF 0x08
#define M_IOEOF   0x10
#define M_IOERR   0x20
#define M_IORW    0x80

/* ---- the file system ---------------------------------------------------------------------- */

static char root[1024] = ".";

void msc_set_root(const char *dir)
{
    snprintf(root, sizeof root, "%s", dir && *dir ? dir : ".");
}

/* a directory's entries, sorted by name without regard to case (dot files left out) */
typedef struct { char **names; int n, cap; } Listing;

static SDL_EnumerationResult SDLCALL list_one(void *ud, const char *dir, const char *name)
{
    Listing *l = ud;
    (void)dir;
    if (name[0] == '.')
        return SDL_ENUM_CONTINUE;
    if (l->n == l->cap) {
        l->cap = l->cap ? l->cap * 2 : 64;
        l->names = realloc(l->names, (size_t)l->cap * sizeof *l->names);
    }
    l->names[l->n++] = SDL_strdup(name);
    return SDL_ENUM_CONTINUE;
}

static int by_name(const void *a, const void *b)
{
    int r = SDL_strcasecmp(*(char *const *)a, *(char *const *)b);
    return r ? r : strcmp(*(char *const *)a, *(char *const *)b);
}

static void list_dir(const char *dir, Listing *l)
{
    memset(l, 0, sizeof *l);
    SDL_EnumerateDirectory(dir, list_one, l);
    if (l->n)
        qsort(l->names, (size_t)l->n, sizeof *l->names, by_name);
}

static void list_free(Listing *l)
{
    int i;
    for (i = 0; i < l->n; i++)
        SDL_free(l->names[i]);
    free(l->names);
    memset(l, 0, sizeof *l);
}

/* the entry of dir whose name is `name` without regard to case, into found; 0 if none */
static int find_entry(const char *dir, const char *name, char *found, size_t cap)
{
    Listing l;
    int i, ok = 0;
    list_dir(dir, &l);
    for (i = 0; i < l.n && !ok; i++)
        if (SDL_strcasecmp(l.names[i], name) == 0) {
            snprintf(found, cap, "%s", l.names[i]);
            ok = 1;
        }
    list_free(&l);
    return ok;
}

/* host path for a DOS name: the drive and leading directories that do not exist under the
   root are skipped (the shipped game lived in C:\MPS\COLONIZE), the rest matched without
   regard to case. Returns 0 when the file exists, 1 when it does not (out holds the path to
   create), -1 when a directory on the way is missing. */
static int host_path(const char *dos, char *out, size_t cap)
{
    char parts[16][64];
    int n = 0, i, k, missing = 0;
    const char *p = dos;
    if (p[0] && p[1] == ':')
        p += 2;
    while (*p && n < 16) {
        size_t len = 0;
        while (*p == '\\' || *p == '/')
            p++;
        while (*p && *p != '\\' && *p != '/' && len < 63)
            parts[n][len++] = *p++;
        parts[n][len] = 0;
        if (len)
            n++;
    }
    /* drop leading components until what is left is found (or only the name remains) */
    for (k = 0; k < n; k++) {
        char cur[1024];
        snprintf(cur, sizeof cur, "%s", root);
        missing = 0;
        for (i = k; i < n; i++) {
            char found[256] = "";
            if (!find_entry(cur, parts[i], found, sizeof found)) {
                size_t j;
                if (i < n - 1) { missing = -1; break; }
                /* a new file: DOS names are upper case */
                for (j = 0; parts[i][j]; j++)
                    found[j] = (char)SDL_toupper((unsigned char)parts[i][j]), found[j + 1] = 0;
                missing = 1;
            }
            snprintf(cur + strlen(cur), sizeof cur - strlen(cur), "/%s", found);
        }
        if (missing >= 0 || k == n - 1) {
            snprintf(out, cap, "%s", cur);
            return missing;
        }
    }
    snprintf(out, cap, "%s/%s", root, n ? parts[n - 1] : "");
    return 1;
}

/* ---- handles ------------------------------------------------------------------------------ */

/* DOS's handles: 0-4 the standard devices (1 and 2 write to the console), 5 up files */
static struct { SDL_IOStream *io; int open; int text; } handles[NHANDLES];

static SDL_IOStream *handle_io(i16 h)
{
    return h >= 5 && h < NHANDLES && handles[h].open ? handles[h].io : NULL;
}

i16 msc_open(const char *name, i16 flags, ...)
{
    char path[1024];
    const char *mode;
    int h, acc = flags & 3, exists;
    SDL_IOStream *io;
    host_path(name, path, sizeof path);
    exists = SDL_GetPathInfo(path, NULL);
    if ((flags & DOS_CREAT) && (flags & DOS_EXCL) && exists)
        return -1;
    if (!exists && !(flags & DOS_CREAT))
        return -1;
    if ((flags & DOS_TRUNC) || !exists)
        mode = acc == DOS_WRONLY ? "wb" : "w+b";
    else if (flags & DOS_APPEND)
        mode = acc == DOS_RDWR ? "a+b" : "ab";
    else
        mode = acc == 0 ? "rb" : "r+b";
    for (h = 5; h < NHANDLES && handles[h].open; h++)
        ;
    if (h == NHANDLES)
        return -1;
    io = SDL_IOFromFile(path, mode);
    if (!io)
        return -1;
    handles[h].io = io;
    handles[h].open = 1;
    handles[h].text = !(flags & DOS_BINARY);
    return (i16)h;
}

static int handle_close(i16 h)
{
    if (h >= 0 && h < 5)
        return 0;
    if (!handle_io(h))
        return -1;
    SDL_CloseIO(handles[h].io);
    handles[h].io = NULL;
    handles[h].open = 0;
    return 0;
}

/* close() -- io.h */
i16 msc_close(i16 h)
{
    return (i16)handle_close(h);
}

i32 msc_filelength(i16 h)
{
    SDL_IOStream *io = handle_io(h);
    Sint64 n = io ? SDL_GetIOSize(io) : -1;
    return n < 0 ? -1 : (i32)n;
}

/* raw reads and writes on a handle, as DOS made them: the count moved, -1 on an error */
static long handle_read(i16 h, void *buf, size_t count)
{
    SDL_IOStream *io = handle_io(h);
    size_t n;
    if (!io)
        return -1;
    if (count == 0)
        return 0;
    n = SDL_ReadIO(io, buf, count);
    return n == 0 && SDL_GetIOStatus(io) == SDL_IO_STATUS_ERROR ? -1 : (long)n;
}

static long handle_write(i16 h, const void *buf, size_t count)
{
    SDL_IOStream *io;
    if (h == 1 || h == 2)
        return (long)fwrite(buf, 1, count, h == 1 ? stdout : stderr);
    io = handle_io(h);
    if (!io)
        return -1;
    return SDL_WriteIO(io, buf, count) == count ? (long)count : -1;
}

/* _dos_read / _dos_write: INT 21h AH=3Fh/40h. Raw bytes, no text translation. 0 or a DOS error. */
u16 msc__dos_read(i16 h, void *buf, u16 count, u16 *done)
{
    long n;
    if (h != 0 && !handle_io(h))
        return 6;                                   /* invalid handle */
    n = h == 0 ? 0 : handle_read(h, buf, count);
    if (n < 0)
        return 5;                                   /* access denied */
    *done = (u16)n;
    return 0;
}

u16 msc__dos_write(i16 h, void *buf, u16 count, u16 *done)
{
    long n;
    if (h != 1 && h != 2 && !handle_io(h))
        return 6;
    n = handle_write(h, buf, count);
    if (n < 0)
        return 5;
    *done = (u16)n;
    return 0;
}

/* ---- stdio -------------------------------------------------------------------------------- */

static MSC_FILE iob[MSC_NFILE];
static char iob_text[MSC_NFILE];           /* opened in text mode */
static char charbuf[MSC_NFILE];            /* an unbuffered stream's one-byte buffer */

static int rt_stream(MSC_FILE *f) { return (int)(f - iob); }

MSC_FILE *msc_fopen(const char *name, const char *mode)
{
    int i, flags = 0, plus = strchr(mode, '+') != NULL, text = strchr(mode, 'b') == NULL;
    i16 h;
    for (i = 3; i < MSC_NFILE && iob[i]._flag; i++)
        ;
    if (i == MSC_NFILE)
        return NULL;
    switch (mode[0]) {
    case 'r': flags = plus ? DOS_RDWR : 0; break;
    case 'w': flags = (plus ? DOS_RDWR : DOS_WRONLY) | DOS_CREAT | DOS_TRUNC; break;
    case 'a': flags = (plus ? DOS_RDWR : DOS_WRONLY) | DOS_CREAT | DOS_APPEND; break;
    default: return NULL;
    }
    if (mode[0] == 'r') {
        char path[1024];
        if (host_path(name, path, sizeof path) != 0)
            return NULL;
    }
    h = msc_open(name, (i16)(flags | (text ? 0 : DOS_BINARY)));
    if (h < 0)
        return NULL;
    memset(&iob[i], 0, sizeof iob[i]);
    iob[i]._file = (char)h;
    iob[i]._flag = (char)(plus ? M_IORW : mode[0] == 'r' ? M_IOREAD : M_IOWRT);
    iob_text[i] = (char)text;
    return &iob[i];
}

static int rt_flush(MSC_FILE *f)
{
    int s = rt_stream(f);
    if ((f->_flag & (M_IOWRT | M_IORW)) && f->_base && f->_ptr > f->_base) {
        size_t n = (size_t)(f->_ptr - f->_base);
        if (handle_write(f->_file, f->_base, n) != (long)n) {
            f->_flag |= M_IOERR;
            return MSC_EOF;
        }
    }
    f->_ptr = f->_base;
    f->_cnt = 0;
    (void)s;
    return 0;
}

i16 msc_fclose(MSC_FILE *f)
{
    int r;
    if (!f || !f->_flag)
        return MSC_EOF;
    r = rt_flush(f);
    if (f->_flag & M_IOMYBUF)
        free(f->_base);
    handle_close(f->_file);
    memset(f, 0, sizeof *f);
    return (i16)r;
}

void msc_setbuf(MSC_FILE *f, char *buf)
{
    rt_flush(f);
    if (f->_flag & M_IOMYBUF)
        free(f->_base);
    f->_flag &= ~(M_IOMYBUF | M_IONBF);
    if (buf)
        f->_base = buf;
    else {
        f->_flag |= M_IONBF;
        f->_base = &charbuf[rt_stream(f)];
    }
    f->_ptr = f->_base;
    f->_cnt = 0;
}

/* refill the buffer; the next byte, or EOF */
static int rt_fill(MSC_FILE *f)
{
    long n;
    size_t size;
    if (f->_flag & (M_IOEOF | M_IOERR) || !handle_io(f->_file))
        return MSC_EOF;
    if (!f->_base) {
        f->_base = malloc(MSC_BUFSIZ);
        f->_flag |= M_IOMYBUF;
    }
    size = f->_flag & M_IONBF ? 1 : MSC_BUFSIZ;
    n = handle_read(f->_file, f->_base, size);
    if (n <= 0) {
        f->_flag |= n < 0 ? M_IOERR : M_IOEOF;
        f->_cnt = 0;
        return MSC_EOF;
    }
    f->_ptr = f->_base;
    f->_cnt = (i16)n;
    return 0;
}

/* one byte as stdio returns it: in text mode, a CR before an LF vanishes and Ctrl-Z ends it */
static int rt_get(MSC_FILE *f)
{
    int c;
    for (;;) {
        if (f->_cnt <= 0 && rt_fill(f) == MSC_EOF)
            return MSC_EOF;
        f->_cnt--;
        c = (unsigned char)*f->_ptr++;
        if (!iob_text[rt_stream(f)])
            return c;
        if (c == 0x1a) {
            f->_flag |= M_IOEOF;
            f->_cnt = 0;
            return MSC_EOF;
        }
        if (c != '\r')
            return c;
        /* a CR: drop it if an LF follows */
        if (f->_cnt <= 0 && rt_fill(f) == MSC_EOF)
            return '\r';
        if (*f->_ptr == '\n')
            continue;
        return '\r';
    }
}

static int rt_put(MSC_FILE *f, int c)
{
    if (!f->_base) {
        if (f->_flag & M_IONBF)
            f->_base = &charbuf[rt_stream(f)];
        else {
            f->_base = malloc(MSC_BUFSIZ);
            f->_flag |= M_IOMYBUF;
        }
        f->_ptr = f->_base;
    }
    if (c == '\n' && iob_text[rt_stream(f)] && rt_put(f, '\r') == MSC_EOF)
        return MSC_EOF;
    *f->_ptr++ = (char)c;
    if (f->_ptr - f->_base >= (f->_flag & M_IONBF ? 1 : MSC_BUFSIZ))
        return rt_flush(f) == MSC_EOF ? MSC_EOF : c;
    return c;
}

/* A null stream is DS:0000 read as a FILE: no flags, so MSC's stdio found nothing to read
   or write there and failed (text_get reads on after closing its file at the end). */

u16 msc_fread(void *p, u16 size, u16 n, MSC_FILE *f)
{
    if (!f)
        return 0;
    unsigned char *d = p;
    u32 total = (u32)size * n, got = 0;
    int c;
    if (!size)
        return 0;
    while (got < total && (c = rt_get(f)) != MSC_EOF)
        d[got++] = (unsigned char)c;
    return (u16)(got / size);
}

u16 msc_fwrite(const void *p, u16 size, u16 n, MSC_FILE *f)
{
    if (!f)
        return 0;
    const unsigned char *s = p;
    u32 total = (u32)size * n, put_ = 0;
    if (!size)
        return 0;
    while (put_ < total && rt_put(f, s[put_]) != MSC_EOF)
        put_++;
    return (u16)(put_ / size);
}

char *msc_fgets(char *s, i16 n, MSC_FILE *f)
{
    if (!f)
        return NULL;
    int i = 0, c = 0;
    if (n <= 0)
        return NULL;
    while (i < n - 1 && (c = rt_get(f)) != MSC_EOF) {
        s[i++] = (char)c;
        if (c == '\n')
            break;
    }
    s[i] = 0;
    return i == 0 && c == MSC_EOF ? NULL : s;
}

static i32 rt_tell(MSC_FILE *f)
{
    SDL_IOStream *io = handle_io(f->_file);
    Sint64 at = io ? SDL_TellIO(io) : -1;
    if (at < 0)
        return -1;
    if (f->_flag & M_IOREAD || (f->_flag & M_IORW && f->_cnt > 0))
        return (i32)(at - f->_cnt);
    return (i32)(at + (f->_base ? f->_ptr - f->_base : 0));
}

static i16 rt_seek(MSC_FILE *f, i32 pos)
{
    SDL_IOStream *io = handle_io(f->_file);
    rt_flush(f);
    f->_flag &= ~(M_IOEOF | M_IOERR);
    f->_cnt = 0;
    f->_ptr = f->_base;
    return io && SDL_SeekIO(io, pos, SDL_IO_SEEK_SET) >= 0 ? 0 : -1;
}

void msc_rewind(MSC_FILE *f) { if (f) rt_seek(f, 0); }

/* fseek: SEEK_SET, SEEK_CUR (from where the program has read to) or SEEK_END */
i16 msc_fseek(MSC_FILE *f, i32 offset, i16 whence)
{
    if (!f)
        return -1;
    i32 base = 0;
    if (whence == 1)
        base = rt_tell(f);
    else if (whence == 2) {
        i32 len = msc_filelength(f->_file);
        if (len < 0)
            return -1;
        base = len;
    }
    return rt_seek(f, base + offset);
}

i16 msc_fgetpos(MSC_FILE *f, msc_fpos_t *pos)
{
    if (!f)
        return -1;
    i32 t = rt_tell(f);
    if (t < 0)
        return -1;
    *pos = t;
    return 0;
}

i16 msc_fsetpos(MSC_FILE *f, const msc_fpos_t *pos) { return f ? rt_seek(f, *pos) : -1; }

i16 msc_remove(const char *name)
{
    char path[1024];
    if (host_path(name, path, sizeof path) != 0)
        return -1;
    return SDL_RemovePath(path) ? 0 : -1;
}

/* MSC's printf formatting: int is 16 bits and long 32, so %d takes an int promoted from i16
   and %ld an i32, which on every target the port builds for is an int: 'l' reads an int.
   %Fs and %Np (far, near) are plain. */
static size_t format(char *out, size_t cap, const char *fmt, va_list ap)
{
    char spec[32];
    size_t o = 0;
    while (*fmt && o + 1 < cap) {
        const char *start = fmt;
        size_t k = 0;
        int r = 0;
        if (*fmt != '%') {
            out[o++] = *fmt++;
            continue;
        }
        fmt++;
        while (*fmt && strchr("-+ #0123456789.", *fmt))
            fmt++;
        while (*fmt == 'l' || *fmt == 'h' || *fmt == 'F' || *fmt == 'N')
            fmt++;
        if (!*fmt)
            break;
        for (const char *q = start; q < fmt && k < sizeof spec - 2; q++)
            if (*q != 'l' && *q != 'F' && *q != 'N')
                spec[k++] = *q;
        spec[k++] = *fmt;
        spec[k] = 0;
        switch (*fmt++) {
        case 's': r = snprintf(out + o, cap - o, spec, va_arg(ap, char *)); break;
        case 'c': case 'd': case 'i': case 'u': case 'x': case 'X': case 'o':
            r = snprintf(out + o, cap - o, spec, va_arg(ap, int)); break;
        case '%': out[o++] = '%'; break;
        default: break;
        }
        if (r > 0)
            o += (size_t)r < cap - o ? (size_t)r : cap - o - 1;
    }
    out[o < cap ? o : cap - 1] = 0;
    return o;
}

/* printf: the editor prints its usage and exit value to the console */
i16 msc_printf(const char *fmt, ...)
{
    char out[1024];
    size_t n;
    va_list ap;
    va_start(ap, fmt);
    n = format(out, sizeof out, fmt, ap);
    va_end(ap);
    fwrite(out, 1, n, stdout);
    return (i16)n;
}

/* sprintf: into the caller's buffer, as MSC's had no limit (the editor's are 80 bytes) */
i16 msc_sprintf(char *buf, const char *fmt, ...)
{
    size_t n;
    va_list ap;
    va_start(ap, fmt);
    n = format(buf, 4096, fmt, ap);
    va_end(ap);
    return (i16)n;
}

/* ---- _dos_findfirst / _dos_findnext (INT 21h AH=4Eh/4Fh) ------------------------------------ */

typedef struct { char dir[1024]; char pattern[16]; long index; u16 attrib; } FindState;
static FindState finds[8];

static int wild_match(const char *pat, const char *name)
{
    /* DOS 8.3 matching: '*' fills the rest of the base or extension, '?' one character */
    char pb[9], pe[4], nb[9], ne[4];
    const char *dot;
    int i;
    for (i = 0; i < 2; i++) {
        const char *s = i ? name : pat;
        char *b = i ? nb : pb, *e = i ? ne : pe;
        size_t lb, le;
        dot = strrchr(s, '.');
        lb = dot ? (size_t)(dot - s) : strlen(s);
        le = dot ? strlen(dot + 1) : 0;
        if (lb > 8 || le > 3)
            return 0;
        memset(b, ' ', 8); b[8] = 0;
        memset(e, ' ', 3); e[3] = 0;
        memcpy(b, s, lb);
        if (dot) memcpy(e, dot + 1, le);
    }
    for (i = 0; i < 11; i++) {
        char p = i < 8 ? pb[i] : pe[i - 8], n = i < 8 ? nb[i] : ne[i - 8];
        if (p == '*') { i = i < 8 ? 7 : 10; continue; }
        if (p != '?' && SDL_toupper((unsigned char)p) != SDL_toupper((unsigned char)n))
            return 0;
    }
    return 1;
}

static u16 find_next_in(struct find_t *f, FindState *st)
{
    Listing l;
    long i;
    list_dir(st->dir, &l);
    for (i = st->index; i < l.n; i++) {
        char full[1300];
        SDL_PathInfo info;
        const char *e = l.names[i];
        int is_dir;
        size_t k;
        if (!wild_match(st->pattern, e))
            continue;
        snprintf(full, sizeof full, "%s/%s", st->dir, e);
        if (!SDL_GetPathInfo(full, &info))
            continue;
        is_dir = info.type == SDL_PATHTYPE_DIRECTORY;
        if (is_dir && !(st->attrib & _A_SUBDIR))
            continue;
        st->index = i + 1;
        memset(f->name, 0, sizeof f->name);
        for (k = 0; e[k] && k < 12; k++)
            f->name[k] = (char)SDL_toupper((unsigned char)e[k]);
        f->attrib = (char)(is_dir ? _A_SUBDIR : _A_ARCH);
        f->size = (i32)info.size;
        f->wr_time = f->wr_date = 0;
        list_free(&l);
        return 0;
    }
    list_free(&l);
    return 18;                                      /* no more files */
}

u16 msc__dos_findfirst(char *spec, u16 attrib, struct find_t *f)
{
    /* the search's state lives in find_t.reserved, as DOS kept it: here, a slot number */
    static int next_slot;
    FindState *st = &finds[next_slot];
    const char *name = spec, *p;
    char dirpart[1024] = "";
    f->reserved[0] = (char)next_slot;
    next_slot = (next_slot + 1) % 8;
    for (p = spec; *p; p++)
        if (*p == '\\' || *p == '/' || *p == ':')
            name = p + 1;
    snprintf(dirpart, sizeof dirpart, "%.*s", (int)(name - spec), spec);
    if (dirpart[0]) {
        if (host_path(dirpart, st->dir, sizeof st->dir) < 0)
            return 3;                               /* path not found */
    } else
        snprintf(st->dir, sizeof st->dir, "%s", root);
    snprintf(st->pattern, sizeof st->pattern, "%s", name);
    st->index = 0;
    st->attrib = attrib;
    return find_next_in(f, st);
}

u16 msc__dos_findnext(struct find_t *f)
{
    return find_next_in(f, &finds[(unsigned char)f->reserved[0] % 8]);
}

/* ---- strings ------------------------------------------------------------------------------ */

/* A null pointer, as the editor sometimes reads one: in DOS, DS:0000, where MSC 6 put eight
   zero bytes and its copyright -- so a null string reads as an empty one (popup_start_box
   measures text_get's NULL at the end of MAPEDIT.TXT's last section). */
static const char ds0[] = "\0\0\0\0\0\0\0\0MS Run-Time Library - Copyright (c) 1990, Microsoft Corp";
#define RD(p) ((p) ? (p) : ds0)

/* MSC's copies measure the source first, then move it forward (rep movsw): defined when the
   two overlap, which the editor relies on (menu_string_width shifts a string down a byte
   with strcpy). The host's are not, so every copy here is a forward move. */
static void fwd(char *d, const char *s, size_t n)
{
    while (n--)
        *d++ = *s++;
}

char *msc_strcat(char *d, const char *s) { s = RD(s); fwd(d + strlen(d), s, strlen(s) + 1); return d; }
char *msc_strncat(char *d, const char *s, u16 n)
{
    size_t k = SDL_strnlen(s, n);
    char *e = d + strlen(d);
    fwd(e, s, k);
    e[k] = 0;
    return d;
}
i16 msc__fmemcmp(const void *a, const void *b, u16 n) { int r = memcmp(a, b, n); return (i16)(r < 0 ? -1 : r > 0); }
char *msc_strcpy(char *d, const char *s) { s = RD(s); fwd(d, s, strlen(s) + 1); return d; }
char *msc_strncpy(char *d, const char *s, u16 n)
{
    size_t k = SDL_strnlen(s, n);
    fwd(d, s, k);
    memset(d + k, 0, n - k);
    return d;
}
u16 msc_strlen(const char *s) { return (u16)strlen(RD(s)); }
i16 msc_strcmp(const char *a, const char *b) { int r = strcmp(RD(a), RD(b)); return (i16)(r < 0 ? -1 : r > 0); }
i16 msc_strncmp(const char *a, const char *b, u16 n) { int r = strncmp(RD(a), RD(b), n); return (i16)(r < 0 ? -1 : r > 0); }
i16 msc_strnicmp(const char *a, const char *b, u16 n)
{
    for (; n; n--, a++, b++) {
        int x = SDL_tolower((unsigned char)*a), y = SDL_tolower((unsigned char)*b);
        if (x != y)
            return (i16)(x < y ? -1 : 1);
        if (!x)
            break;
    }
    return 0;
}
char *msc_strchr(const char *s, i16 c) { return s ? strchr(s, (char)c) : NULL; }
char *msc_strupr(char *s) { char *p; for (p = s; *p; p++) if (*p >= 'a' && *p <= 'z') *p -= 0x20; return s; }
char *msc_strlwr(char *s) { char *p; for (p = s; *p; p++) if (*p >= 'A' && *p <= 'Z') *p += 0x20; return s; }
char *msc__fstrcat(char *d, const char *s) { return msc_strcat(d, s); }
char *msc__fstrcpy(char *d, const char *s) { return msc_strcpy(d, s); }
char *msc__fstrncpy(char *d, const char *s, u16 n) { return msc_strncpy(d, s, n); }
u16 msc__fstrlen(const char *s) { return (u16)strlen(RD(s)); }
i16 msc__fstrncmp(const char *a, const char *b, u16 n) { return msc_strncmp(a, b, n); }
char *msc__fstrchr(const char *s, i16 c) { return strchr(s, (char)c); }
char *msc__fstrrchr(const char *s, i16 c) { return strrchr(s, (char)c); }
char *msc__fstrupr(char *s) { return msc_strupr(s); }
void *msc__fmemcpy(void *d, const void *s, u16 n) { fwd(d, s, n); return d; }
void *msc__fmemset(void *d, i16 c, u16 n) { return memset(d, (unsigned char)c, n); }
void *msc__fmemmove(void *d, const void *s, u16 n) { return memmove(d, s, n); }
i16 msc__fstricmp(const char *a, const char *b)
{
    for (;; a++, b++) {
        int x = SDL_tolower((unsigned char)*a), y = SDL_tolower((unsigned char)*b);
        if (x != y)
            return (i16)(x < y ? -1 : 1);
        if (!x)
            return 0;
    }
}

/* ---- numbers ------------------------------------------------------------------------------ */

i16 msc_atoi(const char *s)
{
    /* MSC: skip white space, an optional sign, digits; the result wraps at 16 bits */
    u16 v = 0;
    int neg = 0;
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r' || *s == '\f' || *s == '\v')
        s++;
    if (*s == '-' || *s == '+')
        neg = *s++ == '-';
    while (*s >= '0' && *s <= '9')
        v = (u16)(v * 10 + (*s++ - '0'));
    return (i16)(neg ? -v : v);
}

static char *to_text(u32 v, int neg, char *s, i16 radix)
{
    char tmp[34];
    int n = 0, i = 0;
    if (radix < 2 || radix > 36) {
        s[0] = 0;
        return s;
    }
    do {
        int d = (int)(v % (u32)radix);
        tmp[n++] = (char)(d < 10 ? '0' + d : 'a' + d - 10);
        v /= (u32)radix;
    } while (v);
    if (neg)
        s[i++] = '-';
    while (n)
        s[i++] = tmp[--n];
    s[i] = 0;
    return s;
}

/* itoa and ltoa write a negative number with a minus sign only in base 10; in any other base
   the value's bits are written as unsigned, 16 bits for itoa, 32 for ltoa */
char *msc_itoa(i16 v, char *s, i16 radix)
{
    if (radix == 10 && v < 0)
        return to_text((u32)(-(i32)v), 1, s, radix);
    return to_text((u16)v, 0, s, radix);
}

char *msc_ltoa(i32 v, char *s, i16 radix)
{
    if (radix == 10 && v < 0)
        return to_text((u32)0 - (u32)v, 1, s, radix);
    return to_text((u32)v, 0, s, radix);
}

/* ---- ctype: MSC's table (index 0 is EOF) -------------------------------------------------- */

u8 _ctype[257] = {
    0,
    /* 00-1F: control; 09-0D also space */
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x28, 0x28, 0x28, 0x28, 0x28, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    /* 20: space (blank), 21-2F punctuation */
    0x48, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    /* 30-39 digits (hex), 3A-40 punctuation */
    0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    /* 40 '@', 41-46 upper hex, 47-5A upper, 5B-60 punctuation */
    0x10, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x10, 0x10, 0x10, 0x10, 0x10,
    /* 60 '`', 61-66 lower hex, 67-7A lower, 7B-7E punctuation, 7F control */
    0x10, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x10, 0x10, 0x10, 0x10, 0x20,
    /* 80-FF: no class */
};
