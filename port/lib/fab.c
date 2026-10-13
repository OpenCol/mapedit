/* fab.c -- pfabexp0/1/2.ASM and pfabcomp.ASM: MicroProse's FAB codec, an LZ77 the MADS
 * engine packs its art with, as pack_a_packet (pack_5.c) calls it.
 *
 *     "FAB", a shift byte (12 in every shipped stream), then 16-bit flag words, least
 *     significant bit first, each fetched the moment the last one runs out, interleaved
 *     with the token bytes:
 *       1            a literal byte follows
 *       0 0 a b      copy 2 + (a<<1|b) bytes from 256 - next byte back
 *       0 1          two bytes L H: distance 10000h - ((H >> (16-shift) | FFh << (shift-8)) << 8 | L),
 *                    length H & ((1 << (16-shift)) - 1): 0 -> one more byte n (0 ends the
 *                    stream, 1 does nothing, else n + 1); k -> k + 2
 *
 * The format is dos-tools' (coldos/fab.py), established by decoding every packed entry of the
 * shipped game exactly. pfabexp0 reads and writes through the packers' callbacks, pfabexp1
 * reads through one and writes to memory, pfabexp2 works memory to memory. pfabcomp writes a
 * stream of literals: valid FAB, which any decoder reads back to the same bytes, though not
 * the bytes MicroProse's packer chose (the editor packs nothing a player keeps).
 */
#include <stdlib.h>
#include <string.h>

#include "msc.h"

typedef u16 (*FFIO)(void *buf, u16 *n);

#define WINDOW 0x10000u
#define CHUNK 0x200u

typedef struct {
    FFIO rd;                    /* the source: a reader, or ... */
    const u8 *mem;              /* ... memory */
    u8 in[CHUNK];
    u16 have, at;
    int eof;
    FFIO wr;                    /* the sink: a writer, or ... */
    u8 *dst;                    /* ... memory */
    u8 ring[WINDOW];            /* what was written, for copies */
    u32 out;
    u8 pend[CHUNK];
    u16 npend;
} Fab;

static Fab F;

static int next(Fab *f)
{
    if (f->mem)
        return *f->mem++;
    if (f->at == f->have) {
        u16 n = CHUNK;
        if (f->eof)
            return -1;
        f->have = f->rd(f->in, &n);
        f->at = 0;
        if (f->have == 0) {
            f->eof = 1;
            return -1;
        }
    }
    return f->in[f->at++];
}

static void flush(Fab *f)
{
    if (f->wr && f->npend) {
        u16 n = f->npend;
        f->wr(f->pend, &n);
        f->npend = 0;
    }
}

static void put(Fab *f, u8 c)
{
    f->ring[f->out & (WINDOW - 1)] = c;
    f->out++;
    if (f->dst)
        *f->dst++ = c;
    else {
        f->pend[f->npend++] = c;
        if (f->npend == CHUNK)
            flush(f);
    }
}

/* 0 when the stream ended properly, 1 for a bad header, 2 for a stream cut short */
static i16 decode(Fab *f)
{
    int shift, b0, b1, c;
    u32 buf;                    /* 17 bits after a refill: the new word above the old word's last bit */
    u16 left = 16;
    u16 ofs_shift, ofs_mask, len_mask;
#define BIT(v) do { \
        if (--left == 0) { \
            if ((b0 = next(f)) < 0 || (b1 = next(f)) < 0) return 2; \
            buf = (u32)(b0 | b1 << 8) << 1 | (buf & 1); \
            left = 16; \
        } \
        (v) = buf & 1; \
        buf >>= 1; \
    } while (0)
    if (next(f) != 'F' || next(f) != 'A' || next(f) != 'B')
        return 1;
    shift = next(f);
    if (shift < 10 || shift > 13)
        return 1;
    ofs_shift = (u16)(16 - shift);
    ofs_mask = (u16)((0xff << (shift - 8)) & 0xff);
    len_mask = (u16)((1 << ofs_shift) - 1);
    if ((b0 = next(f)) < 0 || (b1 = next(f)) < 0)
        return 2;
    buf = (u32)(b0 | b1 << 8);
    for (;;) {
        int bit;
        u32 dist, n, i;
        BIT(bit);
        if (bit) {
            if ((c = next(f)) < 0)
                return 2;
            put(f, (u8)c);
            continue;
        }
        BIT(bit);
        if (!bit) {
            int a, b;
            BIT(a);
            BIT(b);
            n = (u32)(2 + (a << 1 | b));
            if ((c = next(f)) < 0)
                return 2;
            dist = 256u - (u32)c;
        } else {
            int lo = next(f), hi = next(f);
            if (lo < 0 || hi < 0)
                return 2;
            dist = 0x10000u - ((((u32)hi >> ofs_shift | ofs_mask) << 8) | (u32)lo);
            n = (u32)hi & len_mask;
            if (n == 0) {
                if ((c = next(f)) < 0)
                    return 2;
                if (c == 0)
                    break;
                if (c == 1)
                    continue;
                n = (u32)c + 1;
            } else
                n += 2;
        }
        for (i = 0; i < n; i++)
            put(f, f->ring[(f->out - dist) & (WINDOW - 1)]);
    }
#undef BIT
    flush(f);
    return 0;
}

static void start(void)
{
    memset(&F.in, 0, sizeof F.in);
    F.rd = 0; F.mem = 0; F.have = F.at = 0; F.eof = 0;
    F.wr = 0; F.dst = 0; F.out = 0; F.npend = 0;
}

/* PFABEXP0 -- 11a7:0000: reader to writer */
i16 pfabexp0(FFIO rd, FFIO wr, void *work)
{
    (void)work;
    start();
    F.rd = rd;
    F.wr = wr;
    return decode(&F);
}

/* PFABEXP1 -- 11a7:024e: reader to memory */
i16 pfabexp1(FFIO rd, void *dst, void *work)
{
    (void)work;
    start();
    F.rd = rd;
    F.dst = dst;
    return decode(&F);
}

/* PFABEXP2 -- 11a7:045a: memory to memory */
i16 pfabexp2(void *src, void *dst, void *work)
{
    (void)work;
    start();
    F.mem = src;
    F.dst = dst;
    return decode(&F);
}

/* PFABCOMP -- 1168:0006: everything the reader gives, as literals, then the end token.
   The decoder fetches the next flag word when it takes the current word's sixteenth bit,
   before that token's bytes; so the encoder reserves the next word's two bytes the moment a
   word fills, ahead of the bytes of the token that filled it. */
typedef struct { u8 *b; size_t n, cap; size_t word_at; u16 flags, nbits; } Enc;

static void enc_byte(Enc *e, u8 v)
{
    if (e->n == e->cap) {
        e->cap = e->cap ? e->cap * 2 : 4096;
        e->b = realloc(e->b, e->cap);
    }
    e->b[e->n++] = v;
}

static void enc_bit(Enc *e, int bit)
{
    if (bit)
        e->flags |= (u16)(1u << e->nbits);
    if (++e->nbits == 16) {
        e->b[e->word_at] = (u8)e->flags;
        e->b[e->word_at + 1] = (u8)(e->flags >> 8);
        e->flags = 0;
        e->nbits = 0;
        e->word_at = e->n;
        enc_byte(e, 0);
        enc_byte(e, 0);
    }
}

i16 pfabcomp(FFIO rd, FFIO wr, void *work, i16 *type, u16 *dsize)
{
    Enc e;
    u8 in[CHUNK];
    u16 n, got, i;
    size_t at;
    (void)work; (void)type; (void)dsize;
    memset(&e, 0, sizeof e);
    enc_byte(&e, 'F'); enc_byte(&e, 'A'); enc_byte(&e, 'B'); enc_byte(&e, 12);
    e.word_at = e.n;
    enc_byte(&e, 0);
    enc_byte(&e, 0);
    for (;;) {
        n = CHUNK;
        got = rd(in, &n);
        if (got == 0)
            break;
        for (i = 0; i < got; i++) {
            enc_bit(&e, 1);
            enc_byte(&e, in[i]);
        }
    }
    enc_bit(&e, 0);                     /* the end: a long copy of length 0, then n = 0 */
    enc_bit(&e, 1);
    enc_byte(&e, 0);
    enc_byte(&e, 0);
    enc_byte(&e, 0);
    e.b[e.word_at] = (u8)e.flags;
    e.b[e.word_at + 1] = (u8)(e.flags >> 8);
    for (at = 0; at < e.n; at += n) {
        n = (u16)(e.n - at > CHUNK ? CHUNK : e.n - at);
        wr(e.b + at, &n);
    }
    free(e.b);
    return 0;
}
