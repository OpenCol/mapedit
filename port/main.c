/* main.c -- the port's entry: the host, the game directory, then the 1994 editor's main.
 *
 *     mapedit [--game DIR | --choose-game] [--scale N] [--aspect] [--script FILE] [--headless]
 *             [--save-dir DIR] [--improved | --undo N --check-save --export-png [DIR] --new-map-menu]
 *             [--play]
 *             [editor options: -c, -m:FILE]
 *
 * DIR is the player's Colonization for DOS directory (the one holding VICEROY.EXE); by
 * default $MAPEDIT_GAME, else found or asked for (game_dir.c; --choose-game asks again).
 * The editor reads its art, fonts and texts there and writes its maps there, as it did. --script plays a session
 * (platform/host_sdl.c has the commands); --headless opens no window.
 *
 * The improvements (src/include/options.h), all off unless given: --undo N keeps N steps of
 * Undo and Redo, --check-save checks the map before it is saved, --export-png adds Export Map
 * as PNG (written to DIR, else the current folder), --new-map-menu starts with New empty map /
 * Load existing map (also on Create Map); --improved is all four, with 64 steps. (Generate
 * random map needs the game's generator: OpenCol; --test-generator offers it with a stand-in that
 * draws one rectangle of land, sized and coloured by the choices, for the tests.)
 * --save-dir DIR writes maps to DIR instead of the game directory, which is then only read.
 * --play offers Play This Map, for a program that starts the editor: chosen, the editor ends
 * and prints "play NAME.MP".
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

#include "msc.h"
#include "host.h"
#include "game_dir.h"
#include "options.h"

/* --test-generator: Generate random map's stand-in for the game's generator (options.h): ocean,
 * sea lane in the two outer columns each side, and a rectangle of land in the middle whose size
 * follows Land Mass and whose terrain follows Temperature (tundra, plains, savannah) */
static i16 test_generator(const i16 *choice, u8 *terrain, u8 *continent, i16 w, i16 h)
{
    static const u8 land[3] = {0, 2, 5};
    int x, y, rx = 6 + 4 * choice[0], ry = 8 + 5 * choice[0];
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++) {
            u8 t = 0x19;
            if (x < 2 || x >= w - 2)
                t = 0x1a;
            else if (x >= w / 2 - rx && x < w / 2 + rx && y >= h / 2 - ry && y < h / 2 + ry)
                t = land[choice[2]];
            terrain[y * w + x] = t;
            continent[y * w + x] = 0;
        }
    return 0;
}

void msc_set_root(const char *dir);
void msc_set_write_root(const char *dir);

int main(int argc, char **argv)
{
    HostOptions opt;
    const char *game = getenv("MAPEDIT_GAME"), *save_dir = NULL;
    char *args[16];
    int n = 0, i, choose = 0;
    memset(&opt, 0, sizeof opt);
    opt.title = "Colonization Map Editor";
    opt.scale = 3;
    args[n++] = "MAPEDIT.EXE";
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--game") && i + 1 < argc)
            game = argv[++i];
        else if (!strcmp(argv[i], "--scale") && i + 1 < argc)
            opt.scale = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--aspect"))
            opt.aspect = 1;
        else if (!strcmp(argv[i], "--script") && i + 1 < argc)
            opt.script = argv[++i];
        else if (!strcmp(argv[i], "--headless"))
            opt.headless = 1;
        else if (!strcmp(argv[i], "--choose-game"))
            choose = 1;
        else if (!strcmp(argv[i], "--save-dir") && i + 1 < argc)
            save_dir = argv[++i];
        else if (!strcmp(argv[i], "--undo") && i + 1 < argc) {
            int n = atoi(argv[++i]);
            editor_options.undo_levels = (i16)SDL_clamp(n, 0, 1000);
        }
        else if (!strcmp(argv[i], "--check-save"))
            editor_options.save_checks = 1;
        else if (!strcmp(argv[i], "--export-png")) {
            editor_options.export_png = 1;
            if (i + 1 < argc && argv[i + 1][0] != '-')
                editor_options.export_dir = argv[++i];
        } else if (!strcmp(argv[i], "--new-map-menu"))
            editor_options.new_map_menu = 1;
        else if (!strcmp(argv[i], "--test-generator"))
            editor_options.generate_map = test_generator;
        else if (!strcmp(argv[i], "--play"))
            editor_options.play = 1;
        else if (!strcmp(argv[i], "--improved")) {
            editor_options.undo_levels = 64;
            editor_options.save_checks = 1;
            editor_options.export_png = 1;
            editor_options.new_map_menu = 1;
        }
        else if (!strncmp(argv[i], "-psn_", 5))
            ;                           /* macOS's process serial number, from Finder */
        else if (n < 15)
            args[n++] = argv[i];
    }
    args[n] = NULL;
    if (host_init(&opt) != 0) {
        fprintf(stderr, "mapedit: cannot open a window\n");
        return 1;
    }
    {
        const char *given = game;
        game = game_dir_find(given, !opt.headless && !opt.script, choose);
        if (!game) {
            const char *msg = given
                ? "That directory has no Colonization game in it (no VICEROY.PAL)."
                : "The Colonization game files were not found. Run it as\n"
                  "    mapedit --game <the Colonization directory>";
            fprintf(stderr, "mapedit: %s\n", msg);
            if (!opt.headless && given)
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Colonization Map Editor", msg, NULL);
            host_quit();
            return 1;
        }
    }
    msc_set_root(game);
    if (save_dir)
        msc_set_write_root(save_dir);
    editor_main((i16)n, args);         /* the editor leaves through exit() itself, mostly */
    if (editor_options.play_map[0])
        printf("play %s\n", editor_options.play_map);
    host_quit();
    return 0;
}
