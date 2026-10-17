/* options.h -- the port's improvements to the 1994 editor, each off unless asked for.
 *
 * Defined in sys/options.c. With every option off the editor is the 1994 program: the golden
 * and M4 sessions run so (docs/plan.md, M5). port/main.c sets them from the command line; a
 * program the editor is built into sets them before calling editor_main.
 *
 *   undo_levels  0: the original's Undo, one snapshot of the terrain plane taken at the start
 *                of every stroke. n: Undo steps back through the last n changes of all three
 *                saved planes, Redo steps forward again, and loading or creating a map
 *                forgets them (core/editstate.c).
 *   save_checks  before a map is saved, Find Continents runs and the map is checked for what
 *                the game would change or mind; a popup lists what was found and asks whether
 *                to save anyway (editor/checks.c).
 *   export_png   Editor > Export Map as PNG: the whole map at full zoom, written to
 *                export_dir (NULL or "": the current folder), else to export_fallback
 *                (ui/mapimage.c).
 *   play         Editor > Play This Map: saves the map if it has changes and leaves the
 *                editor with its file name in play_map, for the program around it to start a
 *                game on it (editor/mapedit.c).
 */
#ifndef MAPEDIT_OPTIONS_H
#define MAPEDIT_OPTIONS_H

#include "msc.h"

/* The port's own record, laid out naturally wherever it is included: the editor's modules
 * include it under #pragma pack(2), port/main.c and the program around the editor do not. */
#pragma pack(push, 8)
typedef struct {
    i16 undo_levels;
    i16 save_checks;
    i16 export_png;
    const char *export_dir;
    const char *export_fallback;
    i16 play;
    char play_map[16];          /* out: the map to play, or "" */
} EditorOptions;
#pragma pack(pop)

extern EditorOptions editor_options;

#endif
