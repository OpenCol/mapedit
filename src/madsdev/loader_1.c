/* Imported from mapedit-decomp src/madsdev/loader_1 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE loader_1: MAPEDIT.EXE segment 121a (loader_1.c), file 0x137a0-0x13b82, 3 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn loader_set_priority 0x13adc
 * fn loader_close        0x13aee
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl28_76e50).
 *
 * MADSPACK FILES (overlay 28, para 0). DOS-only engine code. A FileIO wraps a stdio FILE
 * holding a "MADSPACK 2.0\x1a" archive: a 0xb0-byte header (12-byte magic, an entry count
 * at +0x28, and 16 ten-byte entries from +0x2a) followed by the data.
 *   - ff_open(how, mode, fh, name) (_fastcall, thunk 181f:2e9e) copies the name to
 *     g_ff_name (DS:0x244c) and opens the file through file_open.
 *       - For reading, it reads the 16-byte head, checks the magic, reads count entries,
 *         seeks past the header and sums the entry sizes into fh->total.
 *       - For writing, it stamps the magic, puts `mode` into entry 0's type and writes a
 *         blank header.
 *     It counts opens in g_ff_opens (DS:0x242e) and returns 0 on success. On failure it
 *     closes the FILE and returns 1.
 *   - loader_set_priority(flag, fh) sets entry 0's flag byte.
 *   - loader_close(fh) (thunk 181f:2eac) resets an in-memory file (kind 1 or 2). Otherwise it
 *     rewinds a file opened for writing, rewrites its header, and fcloses it. It returns 1
 *     when that header write failed.
 *
 * `#pragma optimize("elg")`: loader_set_priority and loader_close match under any pragma with "e".
 *
 *  The size (522) and the overall
 * shape are right with "elg" plus a `volatile` unused local (shipped keeps its dead `= -1`
 * store at bp-4). But register allocation differs: shipped keeps name's offset in si and
 * fh's in di, with `reading` in memory (bp-2, shared later with the loop's i), while ours
 * gives si to fh and di to `reading`. These had no effect: `register` on the parameters,
 * three local declaration orders, testing the field instead of the variable, pads 0-16,
 * and the pragmas "", e, el, eg, l, g, lg.
 * In ff_open fh (16 uses) takes si and reading di; name (2 uses) is not a candidate. A third
 * name use that folds (`|| (unsigned)name < 0`, `fh->open = (unsigned)name < 0`) makes name a
 * candidate -- in di, with fh still in si, and either keeps name live across file_open (so fh
 * spills a 2-word temp) or builds a far-pointer CSE temp of name. Nothing moved fh out of si:
 * `volatile int reading`, `fh->reading = reading; if (fh->reading)`, `char reading`, the
 * file_open test split or `!`-spelled, K&R declarations in either order, `register` on name/fh/
 * both (ignored under elg), `const`/`huge` name, fh read through a cast, a by-value wrapper, or
 * `*(FileIO far **)&fh` (the last three drop fh from the candidates: `les` per use), deleting
 * the loop/else/read branch/tail/kind+x8/x18 statements one at a time, and the pragmas aelg,
 * w-off, t-off, elgs/elgp/elgz, c-off. One observation for the next attempt: shipped frees si at file_open (name
 * dies at its `push si`) and uses si to carry es across that call and the first far_fread
 * (`mov si,es ... mov es,si`), where ours spills es:si to a 2-word temp; so shipped's name
 * must be a register candidate whose range ends there, i.e. a THIRD counted use of name
 * before file_open, not after it. The third use of name is a folded `if ((unsigned)name < 0) goto out;` placed between
 *      `fh->open = 0` and `reading = ...` (after strncpy: 184; after kind/x8 or inside the
 *      file_open call: name stays live across the call and es:si spills).
 *   2. The si/di PAIRING was the `volatile int unused`: with it name always takes di and fh
 *      si, whatever the fold; without it (name's three uses) name takes si and fh di as
 *      shipped. The dead `mov word [bp-4],-1` is kept instead by an aliased local: plain
 *      `int unused; unused = -1; (void)&unused;` or `int unused[1]; unused[0] = -1;` (both
 *      5 bytes). Aliased locals sort LAST among the memory slots, which is the 5 bytes left:
 *      shipped has reading(-2), unused(-4), err(-6); ours reading, err, unused. Volatile
 *      unused sorts above err but flips si/di again; a propagated read (`fh->x8 = unused`
 *      twice) keeps the store above err but makes unused a register candidate (119).
 *   3. `sub dx,dx` for the entry-size product: `(long)(unsigned)(fh->count * sizeof(FFEntry))`
 *      (`10u`, `(unsigned long)` and `unsigned count` all keep the cwd).
 *   4. The loop's segment roles (shipped `mov ds,[bp+0xc]` first, fh via ds:di, entries via
 *      es:si, and no nop after the exit jmp) come from a one-site volatile view on the
 *      reading store: `(*(volatile int far *)&fh->reading = reading) != 0`. A volatile
 *      FIELD does the same; two statements, an element view and a volatile local do not.
 *      No loop spelling (pointer walk, count-down, while, do, temp, huge view) moved it. The aliased `unused` is not what
 * shipped had: an aliased, array, struct or union local sorts LAST however many `&` refs it
 * gets, and aliasing err instead moves err ABOVE reading (67). Shipped's unused is a plain
 * local with a slot of its own. What the probes established:
 *   - Any counted read of a plain `unused` (a propagated `fh->x8 = unused`, a folded
 *     `(unsigned)unused < 0` in any condition, an optimizer-folded `if (unused != -1)`)
 *     gives one state, 328: name loses its register, reading takes si, unused sits at bp-2
 *     sharing with i. Doubling the name test, `if ((unsigned)name < 0 && (unsigned)name
 *     < 0) goto out;` (an `||` or a second statement drops the store), gives name si and fh
 *     di again: 17 bytes, every instruction right, frame 2 bytes short because unused,
 *     reading and i all share bp-2.
 *   - A memory local packs into an earlier slot when the lifetimes are disjoint, and the
 *     lifetime is computed AFTER constant propagation: a propagated read anywhere (x8, x18,
 *     count, total, return, loop body, array index, long argument), `if (unused != -1)`
 *     blocks at any site, dead later defs (`unused = 0` at out:, x18, before return), dead
 *     earlier defs of reading or i, declaration order, do/for wrappers and `>> 16` all leave
 *     the point lifetime (17). `& 0`, `* 0` and unreachable reads after return/goto are not
 *     references at all (store dropped, 428). A REAL read gives shipped's order at once
 *     (`unused = mode; fh->x18 = unused`: reading -2, unused -4, err -6, pairing intact).
 *   - The dead store survives only when a call separates the def from its propagated read
 *     (def after strchr: dropped); a def after strncpy sinks into the next push sequence.
 *   - `i = -1` at the top with `fh->x8 = i` (one variable, two webs): 6 bytes, the slot
 *     takes the union lifetime and sinks below err (bp-6). reading reused as the loop
 *     counter: 8 bytes (unused -2, err -4, reading -6). volatile unused: 181/523.
 *   Model that fits every state: slots are handed out by refs / lifetime length (code
 *   bytes, post-propagation), ties aside, with disjoint packing; shipped's unused is one
 *   store live for roughly 30-130 bytes from the top, i.e. past reading's def, with nothing
 *   reading it. No C spelling found that produces that lifetime without code. The lifetime comes from LATE-FOLDED BLOCKS: a statement whose
 * whole condition is an always-false unsigned test of a parameter, `if ((unsigned)mode < 0)
 * stmt;`, is deleted after slot lifetimes are computed, so a real read inside it extends the
 * variable's range with no code -- provided the read is not constant-propagated first, which a
 * second (also dead) def provides. Source: the doubled name test (R2) plus
 *     if ((unsigned)mode < 0) unused = 0;          before `reading = ...`
 *     if ((unsigned)mode < 0) fh->x8 = unused;     after it (before `fh->kind = 0`)
 * with plain `fh->x8 = -1` and no `(void)&unused`. unused now overlaps reading, so it gets its
 * own slot (bp-4), above err. Equivalent: the read as `fh->x18 = unused`, the read under
 * `(unsigned)how < 0`, or the def inside a `(unsigned)name < 0` block after the doubled test.
 * Needs the doubled name test (single test: 328). Not enough: the read alone (constant, 343,
 * code), the def alone (store dropped, 428), read-then-def in ONE dead block (17: the read sees
 * only the -1 def and is propagated), folded `|| (unsigned)unused < 0` terms inside existing
 * conditions (17: those fold early and extend nothing), the read placed after file_open
 * (`fh->x8 = unused` after or before `fh->x18 = 0`: 338/161, real code survives).
 */

// MSC: #pragma optimize("elg", on)
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;


typedef struct {                /* one directory entry, 10 bytes */
    char type;
    char flag;
    i32 size;
    i32 offset;
} FFEntry;

typedef struct {
    i16 msc_open;
    i16 reading;
    char kind;                  /* 1 or 2: an in-memory file */
    char pad5;
    MSC_FILE *fp;
    i16 x8;
    i16 xa;
    i32 xc;
    i32 x10;
    i32 total;                 /* the entries' sizes summed */
    i16 x18;
    char magic[12];             /* the 0xb0-byte header starts here */
    i16 x26;
    i16 count;
    FFEntry ent[16];
} FileIO;

extern MSC_FILE * file_open(char *name, char *mode);
extern i32 far_fread(void *p, i32 size, i32 n, MSC_FILE *fp);
extern i32 fileio_fwrite_f(void *p, i32 size, i32 n, MSC_FILE *fp);
extern i32 g_ff_opens;                 /* DS:0x242e */
extern char g_ff_name[];                /* DS:0x244c */
/* loader_set_priority -- 121a:033c, 18 bytes */
void loader_set_priority(i16 flag, FileIO *fh)
{
    fh->ent[0].flag = (char)flag;
}
/* loader_close -- 121a:034e, 148 bytes */
i16 loader_close(FileIO *fh)
{
    i16 err;

    err = 0;
    if (fh->msc_open) {
        if (fh->kind == 1 || fh->kind == 2) {
            fh->x10 = 0x4000ffffL;
            fh->xc = 0;
        } else {
            if (fh->reading == 0) {
                msc_rewind(fh->fp);
                err = fileio_fwrite_f(fh->magic, 0xb0L, 1L, fh->fp) == 0;
            }
            msc_fclose(fh->fp);
        }
    }
    fh->msc_open = 0;
    return err;
}

/* DGROUP data that follows this module's literals in the image (DS:0x2426-0x2461): its file state, then ovl28_772ba's I/O hooks. Defined
 * after the last function, so no function's symbol count moves; the values are the shipped ones. */
u8 g_2426[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };/* DS:0x2426 */
i32 g_ff_opens = 0L;                               /* DS:0x242e */
u8 g_2432[26] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };/* DS:0x2432 */
char g_ff_name[14] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };/* DS:0x244c */
void (*g_io_hook_a)(void) = 0;                  /* DS:0x245a */
void (*g_io_hook)(void) = 0;                    /* DS:0x245e */

