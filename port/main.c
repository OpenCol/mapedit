/* main.c -- the port's entry: the host, the game directory, then the 1994 editor's main.
 *
 *     mapedit [--game DIR] [--scale N] [--aspect] [editor options: -c, -m:FILE]
 *
 * DIR is the player's Colonization for DOS directory (the one holding VICEROY.EXE and
 * MAPEDIT.TXT); by default $MAPEDIT_GAME, else the current directory. The editor reads its
 * art, fonts and texts there and writes its maps there, as it did.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "msc.h"
#include "host.h"

void msc_set_root(const char *dir);

int main(int argc, char **argv)
{
    HostOptions opt;
    const char *game = getenv("MAPEDIT_GAME");
    char *args[16];
    int n = 0, i, r;
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
        else if (n < 15)
            args[n++] = argv[i];
    }
    args[n] = NULL;
    if (host_init(&opt) != 0) {
        fprintf(stderr, "mapedit: cannot open a window\n");
        return 1;
    }
    msc_set_root(game ? game : ".");
    r = editor_main((i16)n, args);
    host_quit();
    return r;
}
