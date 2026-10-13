/* game_dir.c -- where the player's Colonization is.
 *
 * The editor needs the game's directory (its palette, fonts, art and maps). In order:
 *   1. the one given (--game, $MAPEDIT_GAME), which must hold the game;
 *   2. the current directory;
 *   3. the program's own directory, and on macOS the one the .app is in -- a copy of the port
 *      dropped into the game's directory starts with a double click;
 *   4. the one the player chose last time (remembered in SDL's preferences directory);
 *   5. ask: a folder dialog, until the player picks a directory with the game or gives up.
 * The game is in a directory when its palette, VICEROY.PAL, is there (in any case).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

#include "game_dir.h"

#define ORG "OpenCol"
#define APP "Colonization Map Editor"

static char found[1024];

static int has_game(const char *dir)
{
    char p[1100];
    snprintf(p, sizeof p, "%s/VICEROY.PAL", dir);
    if (SDL_GetPathInfo(p, NULL))
        return 1;
    snprintf(p, sizeof p, "%s/viceroy.pal", dir);
    return SDL_GetPathInfo(p, NULL);
}

static const char *take(const char *dir)
{
    snprintf(found, sizeof found, "%s", dir);
    return found;
}

/* the remembered choice: one line in <prefs>/game.txt */
static char *remembered_path(void)
{
    char *prefs = SDL_GetPrefPath(ORG, APP), *p;
    if (!prefs)
        return NULL;
    p = SDL_malloc(strlen(prefs) + 16);
    if (p)
        sprintf(p, "%sgame.txt", prefs);
    SDL_free(prefs);
    return p;
}

static void remember(const char *dir)
{
    char *p = remembered_path();
    SDL_IOStream *io;
    if (!p)
        return;
    io = SDL_IOFromFile(p, "wb");
    if (io) {
        SDL_WriteIO(io, dir, strlen(dir));
        SDL_CloseIO(io);
    }
    SDL_free(p);
}

static int recall(char *out, size_t cap)
{
    char *p = remembered_path();
    size_t n = 0;
    void *text = p ? SDL_LoadFile(p, &n) : NULL;
    SDL_free(p);
    if (!text)
        return 0;
    snprintf(out, cap, "%.*s", (int)n, (const char *)text);
    SDL_free(text);
    out[strcspn(out, "\r\n")] = 0;
    return out[0] != 0;
}

/* the folder dialog answers on SDL's event loop */
static struct { int done; char path[1024]; } pick;

static void SDLCALL picked(void *ud, const char *const *files, int filter)
{
    (void)ud; (void)filter;
    if (files && files[0])
        snprintf(pick.path, sizeof pick.path, "%s", files[0]);
    else
        pick.path[0] = 0;
    pick.done = 1;
}

static int ask(const char *why)
{
    const SDL_MessageBoxButtonData buttons[] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Choose Folder..." },
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Quit" },
    };
    SDL_MessageBoxData box;
    int button = 0;
    memset(&box, 0, sizeof box);
    box.flags = SDL_MESSAGEBOX_INFORMATION;
    box.title = APP;
    box.message = why;
    box.numbuttons = 2;
    box.buttons = buttons;
    if (!SDL_ShowMessageBox(&box, &button) || button != 1)
        return 0;
    pick.done = 0;
    SDL_ShowOpenFolderDialog(picked, NULL, NULL, NULL, false);
    while (!pick.done) {
        SDL_Event e;
        while (SDL_PollEvent(&e))
            if (e.type == SDL_EVENT_QUIT)
                return 0;
        SDL_Delay(10);
    }
    return pick.path[0] != 0;
}

const char *game_dir_find(const char *given, int interactive, int choose_again)
{
    char dir[1024];
    const char *base;
    if (given)
        return has_game(given) ? take(given) : NULL;
    if (!choose_again) {
        if (has_game("."))
            return take(".");
        base = SDL_GetBasePath();       /* ends in a separator; in a .app, Contents/Resources/ */
        if (base) {
            if (has_game(base))
                return take(base);
#ifdef __APPLE__
            {
                /* the directory the .app is in: <dir>/X.app/Contents/Resources/ */
                char *app = strstr(base, ".app/Contents/");
                if (app) {
                    snprintf(dir, sizeof dir, "%.*s", (int)(app - base), base);
                    *(strrchr(dir, '/') ? strrchr(dir, '/') : dir) = 0;
                    if (dir[0] && has_game(dir))
                        return take(dir);
                }
            }
#endif
        }
        if (recall(dir, sizeof dir) && has_game(dir))
            return take(dir);
    }
    if (!interactive)
        return NULL;
    if (!ask("Where is Colonization?\n\nChoose the folder of your Colonization for DOS game "
             "(GOG or Steam): the one with VICEROY.EXE in it. The editor reads the game's art "
             "from there, and saves its maps there."))
        return NULL;
    while (!has_game(pick.path)) {
        char why[1400];
        snprintf(why, sizeof why, "%s\n\nThat folder has no Colonization game in it (no "
                 "VICEROY.PAL). Choose the folder with VICEROY.EXE in it.", pick.path);
        if (!ask(why))
            return NULL;
    }
    remember(pick.path);
    return take(pick.path);
}
