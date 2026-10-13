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
 */
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

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
            DIR *d = opendir(cur);
            struct dirent *e;
            char found[256] = "";
            if (d) {
                while ((e = readdir(d)) != NULL)
                    if (strcasecmp(e->d_name, parts[i]) == 0) {
                        snprintf(found, sizeof found, "%s", e->d_name);
                        break;
                    }
                closedir(d);
            }
            if (!found[0]) {
                if (i < n - 1) { missing = -1; break; }
                /* a new file: DOS names are upper case */
                for (size_t j = 0; parts[i][j]; j++)
                    found[j] = (char)toupper((unsigned char)parts[i][j]), found[j + 1] = 0;
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

static struct { int fd; int text; } handles[NHANDLES];
static int handles_ready;

static void handles_init(void)
{
    int i;
    if (handles_ready)
        return;
    for (i = 0; i < NHANDLES; i++)
        handles[i].fd = -1;
    handles[0].fd = 0; handles[1].fd = 1; handles[2].fd = 2;
    handles_ready = 1;
}

static int handle_fd(i16 h)
{
    handles_init();
    return h >= 0 && h < NHANDLES ? handles[h].fd : -1;
}

i16 msc_open(const char *name, i16 flags, ...)
{
    char path[1024];
    int oflags, fd, h;
    handles_init();
    host_path(name, path, sizeof path);
    oflags = (flags & 3) == DOS_WRONLY ? O_WRONLY : (flags & 3) == DOS_RDWR ? O_RDWR : O_RDONLY;
    if (flags & DOS_CREAT) oflags |= O_CREAT;
    if (flags & DOS_TRUNC) oflags |= O_TRUNC;
    if (flags & DOS_EXCL) oflags |= O_EXCL;
    if (flags & DOS_APPEND) oflags |= O_APPEND;
    fd = open(path, oflags, 0644);
    if (fd < 0)
        return -1;
    for (h = 5; h < NHANDLES; h++)
        if (handles[h].fd < 0) {
            handles[h].fd = fd;
            handles[h].text = !(flags & DOS_BINARY);
            return (i16)h;
        }
    close(fd);
    return -1;
}

static int handle_close(i16 h)
{
    int fd = handle_fd(h);
    if (fd < 0)
        return -1;
    if (h >= 5)
        close(fd);
    handles[h].fd = -1;
    return 0;
}

/* close() -- io.h */
i16 msc_close(i16 h)
{
    return (i16)handle_close(h);
}

i32 msc_filelength(i16 h)
{
    struct stat st;
    int fd = handle_fd(h);
    if (fd < 0 || fstat(fd, &st) != 0)
        return -1;
    return (i32)st.st_size;
}

/* _dos_read / _dos_write: INT 21h AH=3Fh/40h. Raw bytes, no text translation. 0 or a DOS error. */
u16 msc__dos_read(i16 h, void *buf, u16 count, u16 *done)
{
    int fd = handle_fd(h);
    ssize_t n;
    if (fd < 0)
        return 6;                                   /* invalid handle */
    n = read(fd, buf, count);
    if (n < 0)
        return 5;                                   /* access denied */
    *done = (u16)n;
    return 0;
}

u16 msc__dos_write(i16 h, void *buf, u16 count, u16 *done)
{
    int fd = handle_fd(h);
    ssize_t n;
    if (fd < 0)
        return 6;
    n = write(fd, buf, count);
    if (n < 0)
        return 5;
    *done = (u16)n;
    return 0;
}

/* ---- stdio -------------------------------------------------------------------------------- */

static MSC_FILE iob[MSC_NFILE];
static char iob_text[MSC_NFILE];           /* opened in text mode */
static char charbuf[MSC_NFILE];            /* an unbuffered stream's one-byte buffer */

static int stream(MSC_FILE *f) { return (int)(f - iob); }

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

static int flush(MSC_FILE *f)
{
    int s = stream(f);
    if ((f->_flag & (M_IOWRT | M_IORW)) && f->_base && f->_ptr > f->_base) {
        int fd = handle_fd(f->_file);
        size_t n = (size_t)(f->_ptr - f->_base);
        if (fd < 0 || write(fd, f->_base, n) != (ssize_t)n) {
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
    r = flush(f);
    if (f->_flag & M_IOMYBUF)
        free(f->_base);
    handle_close(f->_file);
    memset(f, 0, sizeof *f);
    return (i16)r;
}

void msc_setbuf(MSC_FILE *f, char *buf)
{
    flush(f);
    if (f->_flag & M_IOMYBUF)
        free(f->_base);
    f->_flag &= ~(M_IOMYBUF | M_IONBF);
    if (buf)
        f->_base = buf;
    else {
        f->_flag |= M_IONBF;
        f->_base = &charbuf[stream(f)];
    }
    f->_ptr = f->_base;
    f->_cnt = 0;
}

/* refill the buffer; the next byte, or EOF */
static int fill(MSC_FILE *f)
{
    int fd = handle_fd(f->_file);
    ssize_t n;
    size_t size;
    if (f->_flag & (M_IOEOF | M_IOERR) || fd < 0)
        return MSC_EOF;
    if (!f->_base) {
        f->_base = malloc(MSC_BUFSIZ);
        f->_flag |= M_IOMYBUF;
    }
    size = f->_flag & M_IONBF ? 1 : MSC_BUFSIZ;
    n = read(fd, f->_base, size);
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
static int get(MSC_FILE *f)
{
    int c;
    for (;;) {
        if (f->_cnt <= 0 && fill(f) == MSC_EOF)
            return MSC_EOF;
        f->_cnt--;
        c = (unsigned char)*f->_ptr++;
        if (!iob_text[stream(f)])
            return c;
        if (c == 0x1a) {
            f->_flag |= M_IOEOF;
            f->_cnt = 0;
            return MSC_EOF;
        }
        if (c != '\r')
            return c;
        /* a CR: drop it if an LF follows */
        if (f->_cnt <= 0 && fill(f) == MSC_EOF)
            return '\r';
        if (*f->_ptr == '\n')
            continue;
        return '\r';
    }
}

static int put(MSC_FILE *f, int c)
{
    if (!f->_base) {
        if (f->_flag & M_IONBF)
            f->_base = &charbuf[stream(f)];
        else {
            f->_base = malloc(MSC_BUFSIZ);
            f->_flag |= M_IOMYBUF;
        }
        f->_ptr = f->_base;
    }
    if (c == '\n' && iob_text[stream(f)] && put(f, '\r') == MSC_EOF)
        return MSC_EOF;
    *f->_ptr++ = (char)c;
    if (f->_ptr - f->_base >= (f->_flag & M_IONBF ? 1 : MSC_BUFSIZ))
        return flush(f) == MSC_EOF ? MSC_EOF : c;
    return c;
}

u16 msc_fread(void *p, u16 size, u16 n, MSC_FILE *f)
{
    unsigned char *d = p;
    u32 total = (u32)size * n, got = 0;
    int c;
    if (!size)
        return 0;
    while (got < total && (c = get(f)) != MSC_EOF)
        d[got++] = (unsigned char)c;
    return (u16)(got / size);
}

u16 msc_fwrite(const void *p, u16 size, u16 n, MSC_FILE *f)
{
    const unsigned char *s = p;
    u32 total = (u32)size * n, put_ = 0;
    if (!size)
        return 0;
    while (put_ < total && put(f, s[put_]) != MSC_EOF)
        put_++;
    return (u16)(put_ / size);
}

char *msc_fgets(char *s, i16 n, MSC_FILE *f)
{
    int i = 0, c = 0;
    if (n <= 0)
        return NULL;
    while (i < n - 1 && (c = get(f)) != MSC_EOF) {
        s[i++] = (char)c;
        if (c == '\n')
            break;
    }
    s[i] = 0;
    return i == 0 && c == MSC_EOF ? NULL : s;
}

static i32 tell(MSC_FILE *f)
{
    int fd = handle_fd(f->_file);
    off_t at = fd < 0 ? -1 : lseek(fd, 0, SEEK_CUR);
    if (at < 0)
        return -1;
    if (f->_flag & M_IOREAD || (f->_flag & M_IORW && f->_cnt > 0))
        return (i32)(at - f->_cnt);
    return (i32)(at + (f->_base ? f->_ptr - f->_base : 0));
}

static i16 seek(MSC_FILE *f, i32 pos)
{
    int fd = handle_fd(f->_file);
    flush(f);
    f->_flag &= ~(M_IOEOF | M_IOERR);
    f->_cnt = 0;
    f->_ptr = f->_base;
    return fd >= 0 && lseek(fd, pos, SEEK_SET) >= 0 ? 0 : -1;
}

void msc_rewind(MSC_FILE *f) { seek(f, 0); }

i16 msc_fgetpos(MSC_FILE *f, msc_fpos_t *pos)
{
    i32 t = tell(f);
    if (t < 0)
        return -1;
    *pos = t;
    return 0;
}

i16 msc_fsetpos(MSC_FILE *f, const msc_fpos_t *pos) { return seek(f, *pos); }

i16 msc_remove(const char *name)
{
    char path[1024];
    if (host_path(name, path, sizeof path) != 0)
        return -1;
    return unlink(path) == 0 ? 0 : -1;
}

/* printf: the editor prints its usage and exit value to the console */
i16 msc_printf(const char *fmt, ...)
{
    /* MSC's int is 16 bits and its long 32, so %d takes an int promoted from i16 and %ld an
       i32, which on every target the port builds for is an int: read 'l' as int */
    char spec[32], out[1024];
    size_t o = 0;
    va_list ap;
    va_start(ap, fmt);
    while (*fmt && o < sizeof out - 64) {
        const char *start = fmt;
        size_t k = 0;
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
        case 's': o += snprintf(out + o, sizeof out - o, spec, va_arg(ap, char *)); break;
        case 'c': case 'd': case 'i': case 'u': case 'x': case 'X': case 'o':
            o += snprintf(out + o, sizeof out - o, spec, va_arg(ap, int)); break;
        case '%': out[o++] = '%'; break;
        default: break;
        }
        if (o > sizeof out - 64)
            o = sizeof out - 64;
    }
    va_end(ap);
    fwrite(out, 1, o, stdout);
    return (i16)o;
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
        if (p != '?' && toupper((unsigned char)p) != toupper((unsigned char)n))
            return 0;
    }
    return 1;
}

static u16 find_next_in(struct find_t *f, FindState *st)
{
    DIR *d = opendir(st->dir);
    struct dirent *e;
    long i = 0;
    if (!d)
        return 18;                                  /* no more files */
    while ((e = readdir(d)) != NULL) {
        char full[1300];
        struct stat sb;
        if (i++ < st->index || e->d_name[0] == '.')
            continue;
        if (!wild_match(st->pattern, e->d_name))
            continue;
        snprintf(full, sizeof full, "%s/%s", st->dir, e->d_name);
        if (stat(full, &sb) != 0 || (S_ISDIR(sb.st_mode) && !(st->attrib & _A_SUBDIR)))
            continue;
        st->index = i;
        memset(f->name, 0, sizeof f->name);
        for (size_t k = 0; e->d_name[k] && k < 12; k++)
            f->name[k] = (char)toupper((unsigned char)e->d_name[k]);
        f->attrib = (char)(S_ISDIR(sb.st_mode) ? _A_SUBDIR : _A_ARCH);
        f->size = (i32)sb.st_size;
        f->wr_time = f->wr_date = 0;
        closedir(d);
        return 0;
    }
    closedir(d);
    return 18;
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

char *msc_strcat(char *d, const char *s) { return strcat(d, s); }
char *msc_strcpy(char *d, const char *s) { return strcpy(d, s); }
char *msc_strncpy(char *d, const char *s, u16 n) { return strncpy(d, s, n); }
u16 msc_strlen(const char *s) { return (u16)strlen(s); }
i16 msc_strcmp(const char *a, const char *b) { int r = strcmp(a, b); return (i16)(r < 0 ? -1 : r > 0); }
i16 msc_strncmp(const char *a, const char *b, u16 n) { int r = strncmp(a, b, n); return (i16)(r < 0 ? -1 : r > 0); }
i16 msc_strnicmp(const char *a, const char *b, u16 n)
{
    for (; n; n--, a++, b++) {
        int x = tolower((unsigned char)*a), y = tolower((unsigned char)*b);
        if (x != y)
            return (i16)(x < y ? -1 : 1);
        if (!x)
            break;
    }
    return 0;
}
char *msc_strchr(const char *s, i16 c) { return strchr(s, (char)c); }
char *msc_strupr(char *s) { char *p; for (p = s; *p; p++) if (*p >= 'a' && *p <= 'z') *p -= 0x20; return s; }
char *msc_strlwr(char *s) { char *p; for (p = s; *p; p++) if (*p >= 'A' && *p <= 'Z') *p += 0x20; return s; }
char *msc__fstrcat(char *d, const char *s) { return strcat(d, s); }
char *msc__fstrcpy(char *d, const char *s) { return strcpy(d, s); }
char *msc__fstrncpy(char *d, const char *s, u16 n) { return strncpy(d, s, n); }
u16 msc__fstrlen(const char *s) { return (u16)strlen(s); }
i16 msc__fstrncmp(const char *a, const char *b, u16 n) { return msc_strncmp(a, b, n); }
char *msc__fstrchr(const char *s, i16 c) { return strchr(s, (char)c); }
char *msc__fstrrchr(const char *s, i16 c) { return strrchr(s, (char)c); }
char *msc__fstrupr(char *s) { return msc_strupr(s); }
void *msc__fmemcpy(void *d, const void *s, u16 n) { return memcpy(d, s, n); }
void *msc__fmemset(void *d, i16 c, u16 n) { return memset(d, (unsigned char)c, n); }

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
