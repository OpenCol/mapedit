# Architecture

The editor is the 1994 program's C, reorganised into four layers. Each layer may use the ones
below it and never the ones above; `tools/layers.py` checks that (it runs in CTest).

```
editor   src/editor   the commands and the main loop        (the controller)
ui       src/ui       drawing, widgets, the screens         (the views)
core     src/core     the map, its files, the editing state (the model)
sys      src/sys      files, memory, packing, input, the clock, the screen surfaces
         port/        MSC 6's run-time (msc_*), the program's entry, the game folder, PNG files
         platform/    SDL3: the window, the keyboard and mouse, the clock
```

Every function, global and record is declared once, in `src/include/<subsystem>.h`, from its
definition. `tools/proto_audit.py` checks that no module declares anything itself (also in
CTest): in 1994 each module declared what it used, and disagreements -- an argument order,
a record read through bytes -- compiled silently.

## sys

| File | |
| --- | --- |
| `bios.c` | the BIOS data area: the shift keys, the tick count |
| `btype.c` | numbers in text: binary, decimal, hexadecimal |
| `buffer.c` | 2D byte arrays (`DosPort`): pixels, rectangles, lines, saving what a window covers |
| `error.c` | error reports, the fatal shutdown, console output |
| `fab.c` | MicroProse's FAB codec (LZ77) |
| `fileio.c` | DOS file names in the game folder, block reads and writes, text lines |
| `keys.c` | the keyboard queue (scan codes, Alt key folding) |
| `loader.c` | MADSPACK files |
| `mem.c` | named memory blocks and the arenas carved from them |
| `mouse.c` | the mouse driver, the software cursor, button strokes |
| `pack.c` | the stream plumbing around the codec |
| `screen.c` | the screens the editor draws into (work, original, video) |
| `sort.c`, `strings.c`, `text.c` | sorting; the string store; the text files (MAPEDIT.TXT, NAMES.TXT) |
| `options.c` | the port's improvements, all off (`options.h`) |
| `sound.c` | the three sound calls the editor makes (it plays nothing) |
| `timer.c` | the 600 Hz and 60 Hz clocks; `timer_idle`, where the UI loops rest |
| `video.c` | mode 13h: the screen and its address |

## core

| File | |
| --- | --- |
| `map.c` | the map's planes: terrain, features, continents; what stands on a square |
| `mapfile.c` | `.MP` files: loading, saving, a blank map, the old forest codes (`forest_fix`) |
| `terrain.c` | the terrain table from NAMES.TXT |
| `continents.c` | Find Continents |
| `editstate.c` | the cursor, the selection and how it paints, the brush, coast protection, undo (and the port's undo history) |
| `distance.c`, `compass.c` | minimax; the eight directions |
| `game.c` | the game's records and empty stand-ins for game functions the shared modules call |

## ui

| File | |
| --- | --- |
| `mapview.c` | the map view: geometry, terrain with coasts and seams, the reveal, the cursor square |
| `mapimage.c` | port: the whole map drawn into a picture, for Export Map as PNG |
| `minimap.c` | the minimap |
| `infopanel.c` | the information window beside the map |
| `tilepicker.c` | Map Tile Select |
| `popup.c`, `menu.c` | popups and menus |
| `write.c`, `font.c`, `theme.c` | text, fonts, the editor's colours |
| `sprite.c`, `tile.c`, `assets.c` | sprite series, tiles, the art the editor loads |
| `palette.c`, `hotspot.c` | the palette (colour allocation, the DAC); click rectangles |

## editor

| File | |
| --- | --- |
| `mapedit.c` | startup, the command line, the menu bar's commands, the file menu |
| `input.c` | the main loop, keys, mouse strokes over the map and the minimap |
| `edit.c` | painting, picking up, the brush, the cursor and the view (the commands that redraw) |
| `checks.c` | port: the checks before saving |

## Checks

| | needs the game | what it says |
| --- | --- | --- |
| `ctest` (self-tests, layers, declarations) | no | the run-time and host behave; the layers and declarations hold |
| `tools/golden.py` (in `ctest` with `$MAPEDIT_GAME`) | yes | 18 sessions give the same screens and files as recorded (the 4 `x_*` ones with the port's improvements on) |
| `tests/oracle` (with `$MAPEDIT_EXE`) | the EXE | the C for the 1994 assembly computes what the original does |
| `tools/m4.py` | yes, and DOSBox-X | the port saves the same files as the original under DOSBox-X |

Maintenance tools: `tools/dead_code.py` lists what the program never references (macOS: the
linker's dead stripping); `tools/remove_symbols.py` removes functions and globals with their
comments and declarations; `tools/same_code.py` proves a change touched comments only.
