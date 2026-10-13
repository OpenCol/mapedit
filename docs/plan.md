# Plan: the Colonization map editor on modern systems

`MAPEDIT.EXE`, the map editor MicroProse shipped with *Colonization* for DOS in 1994, rebuilt
from its recovered source ([mapedit-decomp](../../mapedit-decomp)) as a native program for
Windows, macOS and Linux, on SDL3.

Two stages:
- **Faithful.** The port behaves exactly like the DOS program. With the same input it shows the
  same screens and writes the same `.MP` files.
- **Improved.** Changes on top of that baseline, each one optional and each one listed.

## Starting point

mapedit-decomp holds the editor as C that Microsoft C 6.00A compiles to the shipped bytes, one
file per function, under the original names (the EXE kept its linker's symbol table). That is
the specification: the port changes how the C is compiled and what it runs on, never what it
does.

The program has three layers, and the port treats each one differently:

| Layer | Modules | In the port |
| --- | --- | --- |
| The editor | `mapedit`, `map*`, `popup`, `menu`, `text`, `write`, `tile`, `terrain`, `me_mini`, ... | converted from the decomp, maintained here |
| MicroProse's library, C | `buffer_*`, `sprite_*`, `font_*`, `pal_*`, `pack_*`, `loader_*`, `fileio_*`, `hspot_*`, ... | converted; inline `_asm` rewritten in C (`port/lib`) |
| The machine | the 13 asm modules (`mouse_1`, `keys_4`, `timer_3`, `video_1/2`, `sound_1/2`, `stack_1`, `fileio_c`, PKWARE's `pfab*`), the memory managers (`mem_*`, `ems_*`, `xms_*`, `himem_*`), and every DOS/BIOS call | replaced by `platform/`, under the same names |

## The conversion

`src/` was converted once from mapedit-decomp (16-bit types kept, `far`/`near` and calling
conventions dropped, inline assembly rewritten in C under `port/lib/`) and is maintained
here since: [origin.md](origin.md) has the details. 16-bit arithmetic that wrapped in 1994
must still wrap; that remains the main thing to watch when changing it.

## The platform layer (platform/)

The C implementations of what the asm modules and DOS did, keeping the original entry points:
- **video**: a 320×200 8-bit framebuffer standing in for VGA mode 13h, with the 6-bit DAC
  palette, shown in a window scaled by an integer factor (`video_init`, `video_update`,
  `VIDEO_ADDRESS`, the `mcga_*` palette functions);
- **mouse**: the INT 33h driver and MicroProse's software cursor (`mouse_*`: position, buttons,
  the cursor drawn into the work buffer and wiped, view ports, freeze/thaw);
- **keyboard**: the INT 9 handler and the BIOS queue (`KEYS_*`, `keys_*`): SDL key events
  become the scan code / ASCII pairs the editor reads;
- **timer**: the INT 8 handler (`timer_install`, `timer_read`, `timer_read_600`), from a
  monotonic clock, or from a virtual clock that counts reads (for the gates);
- **files**: DOS file names in the game directory, found whatever their case;
- **memory**: `mem_get` and friends on `malloc`; EMS, XMS and upper memory report "absent";
- **decompression**: MicroProse's FAB codec for the game's packed art (`port/lib/fab.c`);
- **sound**: none (the editor makes none).

## Milestones and gates

| | Milestone | Gate (a command that exits 0) |
| --- | --- | --- |
| M0 | this repository, the SDL3 host, CI | `ctest`: the host self-test |
| M1 | every imported module compiles | warning-free on clang, gcc and MSVC |
| M2 | it links | no unresolved symbol |
| M3 | it starts, shows the editor and edits a map | a scripted session runs to the end |
| M4 | **faithful** | the same input script, run in the original (DOSBox-X) and in the port, writes byte-identical `.MP` files, and the screens match at each checkpoint |
| M5 | improvements | each one behind an option, M4 still green with all of them off |

Status (2026-10-13): Windows x64 builds (MinGW-w64) and passes every M4 session under Wine;
macOS likewise natively. M2 done; M3's session runs (`--script`: the editor opens AMER2.MP,
paints, saves). M4 has its harness, `tools/m4.py`: a keyboard session (`tests/m4/*.txt`,
in DOSBox-X's AUTOTYPE key names) is typed into the original under DOSBox-X and played
into the port, each in a fresh copy of the game directory, and every file is compared
afterwards. Screens are not compared yet: DOSBox-X takes no screenshot a session can ask
for, and the port's `shot` has nothing to be held against.

Gate M4 is the contract. The sessions are keyboard-only because AUTOTYPE types keys and
moves no mouse; the editor drives everything from the keyboard (the keypad is its mouse).
Time is not matched: AUTOTYPE paces keys in real time, and nothing the editor writes to a
file depends on the clock.

## Improvements (M5), candidates

- window scaling and fullscreen; aspect-correct 4:3 presentation;
- the mouse wheel for zoom (`Z`/`X`), dragging the view;
- more undo levels (the original has one);
- native open/save dialogs beside the original's file list;
- maps larger than the original's 12,000-square limit, flagged as unreadable by the 1994 game;
- an export of the map as a PNG.
