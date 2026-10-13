/* host_sdl.c -- host.h on SDL3.
 *
 * The framebuffer is the editor's VGA memory: 320x200 bytes, each an index into a 256-entry
 * DAC of 6-bit components, as mode 13h has it. host_present() expands it through the DAC into
 * a streaming texture and shows it scaled (nearest neighbour, whole multiples).
 *
 * The keyboard queue holds what the BIOS keyboard buffer held: for each key press, the scan
 * code (set 1) in the high byte and the ASCII code in the low byte, 0 for keys without one.
 * host_key_translate() makes those from SDL scan codes the way a US keyboard and the BIOS
 * did, rather than from SDL's text input, so a key always gives the same code whatever the
 * system's layout or input method.
 */
#include "host.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QUEUE 32                    /* the BIOS buffer held 15; the editor drains it often */

uint8_t host_vga[0x10000];

static struct {
    HostOptions opt;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint8_t dac[256 * 3];
    uint32_t rgb[HOST_W * HOST_H];
    uint16_t keys[QUEUE];
    int key_head, key_tail;
    int shift;
    int mouse_x, mouse_y, buttons;
    int quit;
    uint64_t t0;
    int virtual_time;
    uint32_t virtual_ms;
    int dirty;
    uint32_t last_present;
    HostMouseHandler mouse_handler;
    HostKeyHook key_hook;
    FILE *script;
    uint32_t script_due;            /* the next command runs at this host_ms() */
    int script_held;                /* shift flags `hold` keeps down */
} H;

/* ---- keyboard ---------------------------------------------------------------------------- */

typedef struct { uint8_t scan, plain, shifted; } KeyDef;

/* SDL scan code -> PC scan code (set 1) and the ASCII a US keyboard gives, plain and shifted */
static const KeyDef *keydef(int sc)
{
    static const struct { int sdl; KeyDef k; } table[] = {
        {SDL_SCANCODE_ESCAPE, {0x01, 0x1b, 0x1b}},
        {SDL_SCANCODE_1, {0x02, '1', '!'}}, {SDL_SCANCODE_2, {0x03, '2', '@'}},
        {SDL_SCANCODE_3, {0x04, '3', '#'}}, {SDL_SCANCODE_4, {0x05, '4', '$'}},
        {SDL_SCANCODE_5, {0x06, '5', '%'}}, {SDL_SCANCODE_6, {0x07, '6', '^'}},
        {SDL_SCANCODE_7, {0x08, '7', '&'}}, {SDL_SCANCODE_8, {0x09, '8', '*'}},
        {SDL_SCANCODE_9, {0x0a, '9', '('}}, {SDL_SCANCODE_0, {0x0b, '0', ')'}},
        {SDL_SCANCODE_MINUS, {0x0c, '-', '_'}}, {SDL_SCANCODE_EQUALS, {0x0d, '=', '+'}},
        {SDL_SCANCODE_BACKSPACE, {0x0e, 0x08, 0x08}}, {SDL_SCANCODE_TAB, {0x0f, 0x09, 0x00}},
        {SDL_SCANCODE_Q, {0x10, 'q', 'Q'}}, {SDL_SCANCODE_W, {0x11, 'w', 'W'}},
        {SDL_SCANCODE_E, {0x12, 'e', 'E'}}, {SDL_SCANCODE_R, {0x13, 'r', 'R'}},
        {SDL_SCANCODE_T, {0x14, 't', 'T'}}, {SDL_SCANCODE_Y, {0x15, 'y', 'Y'}},
        {SDL_SCANCODE_U, {0x16, 'u', 'U'}}, {SDL_SCANCODE_I, {0x17, 'i', 'I'}},
        {SDL_SCANCODE_O, {0x18, 'o', 'O'}}, {SDL_SCANCODE_P, {0x19, 'p', 'P'}},
        {SDL_SCANCODE_LEFTBRACKET, {0x1a, '[', '{'}}, {SDL_SCANCODE_RIGHTBRACKET, {0x1b, ']', '}'}},
        {SDL_SCANCODE_RETURN, {0x1c, 0x0d, 0x0d}}, {SDL_SCANCODE_KP_ENTER, {0x1c, 0x0d, 0x0d}},
        {SDL_SCANCODE_A, {0x1e, 'a', 'A'}}, {SDL_SCANCODE_S, {0x1f, 's', 'S'}},
        {SDL_SCANCODE_D, {0x20, 'd', 'D'}}, {SDL_SCANCODE_F, {0x21, 'f', 'F'}},
        {SDL_SCANCODE_G, {0x22, 'g', 'G'}}, {SDL_SCANCODE_H, {0x23, 'h', 'H'}},
        {SDL_SCANCODE_J, {0x24, 'j', 'J'}}, {SDL_SCANCODE_K, {0x25, 'k', 'K'}},
        {SDL_SCANCODE_L, {0x26, 'l', 'L'}}, {SDL_SCANCODE_SEMICOLON, {0x27, ';', ':'}},
        {SDL_SCANCODE_APOSTROPHE, {0x28, '\'', '"'}}, {SDL_SCANCODE_GRAVE, {0x29, '`', '~'}},
        {SDL_SCANCODE_BACKSLASH, {0x2b, '\\', '|'}},
        {SDL_SCANCODE_Z, {0x2c, 'z', 'Z'}}, {SDL_SCANCODE_X, {0x2d, 'x', 'X'}},
        {SDL_SCANCODE_C, {0x2e, 'c', 'C'}}, {SDL_SCANCODE_V, {0x2f, 'v', 'V'}},
        {SDL_SCANCODE_B, {0x30, 'b', 'B'}}, {SDL_SCANCODE_N, {0x31, 'n', 'N'}},
        {SDL_SCANCODE_M, {0x32, 'm', 'M'}}, {SDL_SCANCODE_COMMA, {0x33, ',', '<'}},
        {SDL_SCANCODE_PERIOD, {0x34, '.', '>'}}, {SDL_SCANCODE_SLASH, {0x35, '/', '?'}},
        {SDL_SCANCODE_KP_MULTIPLY, {0x37, '*', '*'}}, {SDL_SCANCODE_SPACE, {0x39, ' ', ' '}},
        {SDL_SCANCODE_F1, {0x3b, 0, 0}}, {SDL_SCANCODE_F2, {0x3c, 0, 0}},
        {SDL_SCANCODE_F3, {0x3d, 0, 0}}, {SDL_SCANCODE_F4, {0x3e, 0, 0}},
        {SDL_SCANCODE_F5, {0x3f, 0, 0}}, {SDL_SCANCODE_F6, {0x40, 0, 0}},
        {SDL_SCANCODE_F7, {0x41, 0, 0}}, {SDL_SCANCODE_F8, {0x42, 0, 0}},
        {SDL_SCANCODE_F9, {0x43, 0, 0}}, {SDL_SCANCODE_F10, {0x44, 0, 0}},
        {SDL_SCANCODE_HOME, {0x47, 0, 0}}, {SDL_SCANCODE_UP, {0x48, 0, 0}},
        {SDL_SCANCODE_PAGEUP, {0x49, 0, 0}}, {SDL_SCANCODE_KP_MINUS, {0x4a, '-', '-'}},
        {SDL_SCANCODE_LEFT, {0x4b, 0, 0}}, {SDL_SCANCODE_KP_5, {0x4c, 0, '5'}},
        {SDL_SCANCODE_RIGHT, {0x4d, 0, 0}}, {SDL_SCANCODE_KP_PLUS, {0x4e, '+', '+'}},
        {SDL_SCANCODE_END, {0x4f, 0, 0}}, {SDL_SCANCODE_DOWN, {0x50, 0, 0}},
        {SDL_SCANCODE_PAGEDOWN, {0x51, 0, 0}}, {SDL_SCANCODE_INSERT, {0x52, 0, 0}},
        {SDL_SCANCODE_DELETE, {0x53, 0, 0}},
        {SDL_SCANCODE_KP_7, {0x47, 0, '7'}}, {SDL_SCANCODE_KP_8, {0x48, 0, '8'}},
        {SDL_SCANCODE_KP_9, {0x49, 0, '9'}}, {SDL_SCANCODE_KP_4, {0x4b, 0, '4'}},
        {SDL_SCANCODE_KP_6, {0x4d, 0, '6'}}, {SDL_SCANCODE_KP_1, {0x4f, 0, '1'}},
        {SDL_SCANCODE_KP_2, {0x50, 0, '2'}}, {SDL_SCANCODE_KP_3, {0x51, 0, '3'}},
        {SDL_SCANCODE_KP_0, {0x52, 0, '0'}}, {SDL_SCANCODE_KP_PERIOD, {0x53, 0, '.'}},
    };
    size_t i;
    for (i = 0; i < sizeof table / sizeof table[0]; i++)
        if (table[i].sdl == sc)
            return &table[i].k;
    return NULL;
}

/* The BIOS's INT 16h, AH=00h code for a key press: scan code << 8 | ASCII. Function keys
 * move to their own scan codes when shifted (F1: 3Bh, shift 54h, ctrl 5Eh, alt 68h); a
 * control letter is its ASCII control code; an alt key has ASCII 0. 0: no code. */
uint16_t host_key_translate(int sc, int shift, int ctrl, int alt)
{
    const KeyDef *k = keydef(sc);
    int scan, ascii;
    if (!k)
        return 0;
    scan = k->scan;
    if (scan >= 0x3b && scan <= 0x44) {
        if (alt) scan += 0x68 - 0x3b;
        else if (ctrl) scan += 0x5e - 0x3b;
        else if (shift) scan += 0x54 - 0x3b;
        return (uint16_t)(scan << 8);
    }
    ascii = shift ? k->shifted : k->plain;
    if (alt) {
        if (scan >= 0x02 && scan <= 0x0d)
            scan += 0x78 - 0x02;           /* alt-1 .. alt-= are 78h..83h */
        ascii = 0;
    } else if (ctrl) {
        if (ascii >= 'a' && ascii <= 'z') ascii -= 0x60;
        else if (ascii >= 'A' && ascii <= 'Z') ascii -= 0x40;
        else if (scan == 0x1c) ascii = 0x0a;
        else if (scan == 0x0e) ascii = 0x7f;
        else ascii = 0;
    }
    return (uint16_t)(scan << 8 | (ascii & 0xff));
}

static void key_push(uint16_t code)
{
    int next = (H.key_tail + 1) % QUEUE;
    if (next == H.key_head)
        return;                             /* full: the BIOS beeped and dropped it */
    H.keys[H.key_tail] = code;
    H.key_tail = next;
}

int host_key_ready(void) { host_pump(); return H.key_head != H.key_tail; }

uint16_t host_key_get(void)
{
    uint16_t code;
    host_pump();
    if (H.key_head == H.key_tail)
        return 0;
    code = H.keys[H.key_head];
    H.key_head = (H.key_head + 1) % QUEUE;
    return code;
}

void host_key_flush(void) { host_pump(); H.key_head = H.key_tail = 0; }
int host_shift_flags(void) { host_pump(); return H.shift; }

/* ---- events ------------------------------------------------------------------------------ */

void host_set_mouse_handler(HostMouseHandler fn) { H.mouse_handler = fn; }
void host_set_key_hook(HostKeyHook fn) { H.key_hook = fn; }
void host_vram_touched(void) { H.dirty = 1; }

static void present_if_due(void);

static int is_keypad(SDL_Scancode sc)
{
    return sc >= SDL_SCANCODE_KP_DIVIDE && sc <= SDL_SCANCODE_KP_PERIOD;
}

static void shift_from(SDL_Keymod m)
{
    H.shift = (m & SDL_KMOD_RSHIFT ? HOST_SHIFT_RIGHT : 0) | (m & SDL_KMOD_LSHIFT ? HOST_SHIFT_LEFT : 0)
            | (m & SDL_KMOD_CTRL ? HOST_CTRL : 0) | (m & SDL_KMOD_ALT ? HOST_ALT : 0);
}

/* ---- scripted sessions -------------------------------------------------------------------
 *
 * --script FILE plays a session instead of the user's input, one command a line, each when
 * the one before it is done (# starts a comment):
 *
 *     wait MS                  let MS milliseconds pass
 *     key [shift+|ctrl+|alt+]NAME   press and release a key; NAME is SDL's (Return, F1, A, Keypad 8)
 *     type TEXT                the keys of TEXT (a US keyboard)
 *     move X Y                 the mouse to screen pixel X,Y
 *     down [left|right]        press a button; up releases it
 *     click X Y [left|right]   move, press, wait 50 ms, release
 *     hold [shift|ctrl|alt ...]  keep those keys down (for clicks); hold alone releases them
 *     shot FILE                the screen as it is now, a 320x200 BMP
 *     exit                     end the program
 *
 * Every command is logged to stderr with the time it ran. */

static void script_mouse(int x, int y, int buttons)
{
    H.mouse_x = x < 0 ? 0 : x >= HOST_W ? HOST_W - 1 : x;
    H.mouse_y = y < 0 ? 0 : y >= HOST_H ? HOST_H - 1 : y;
    H.buttons = buttons;
    if (H.mouse_handler)
        H.mouse_handler(H.mouse_x, H.mouse_y, H.buttons);
}

static void script_key(SDL_Scancode sc, int shift, int ctrl, int alt)
{
    const KeyDef *k = keydef(sc);
    uint16_t code;
    H.shift = (shift ? HOST_SHIFT_LEFT : 0) | (ctrl ? HOST_CTRL : 0) | (alt ? HOST_ALT : 0) | H.script_held;
    if (!(k && H.key_hook && H.key_hook(k->scan, 1, is_keypad(sc)))) {
        code = host_key_translate(sc, shift, ctrl, alt);
        if (code)
            key_push(code);
    }
    if (k && H.key_hook)
        H.key_hook(k->scan, 0, is_keypad(sc));
    H.shift = H.script_held;
}

static void write_bmp(const char *path)
{
    FILE *f = fopen(path, "wb");
    uint8_t h[54] = {'B', 'M'};
    uint32_t size = 54 + HOST_W * HOST_H * 3;
    int x, y;
    if (!f) {
        fprintf(stderr, "script: cannot write %s\n", path);
        return;
    }
    host_present();
#define LE32(o, v) (h[o] = (uint8_t)(v), h[o + 1] = (uint8_t)((v) >> 8), h[o + 2] = (uint8_t)((v) >> 16), h[o + 3] = (uint8_t)((v) >> 24))
    LE32(2, size); LE32(10, 54); LE32(14, 40); LE32(18, HOST_W); LE32(22, HOST_H);
    h[26] = 1; h[28] = 24; LE32(34, HOST_W * HOST_H * 3);
#undef LE32
    fwrite(h, 1, 54, f);
    for (y = HOST_H - 1; y >= 0; y--)
        for (x = 0; x < HOST_W; x++) {
            uint32_t c = H.rgb[y * HOST_W + x];
            uint8_t px[3] = {(uint8_t)c, (uint8_t)(c >> 8), (uint8_t)(c >> 16)};
            fwrite(px, 1, 3, f);
        }
    fclose(f);
}

static int button_of(const char *s)
{
    return s && !strcmp(s, "right") ? HOST_BUTTON_RIGHT : HOST_BUTTON_LEFT;
}

static void script_step(void)
{
    char line[256], *cmd, *arg;
    static int clicking;                    /* a click's release, 50 ms after its press */
    while (H.script && host_ms() >= H.script_due) {
        if (clicking) {
            script_mouse(H.mouse_x, H.mouse_y, H.buttons & ~clicking);
            clicking = 0;
            continue;
        }
        if (!fgets(line, sizeof line, H.script)) {
            fclose(H.script);
            H.script = NULL;
            fprintf(stderr, "script: end\n");
            return;
        }
        line[strcspn(line, "\r\n")] = 0;
        cmd = line + strspn(line, " \t");
        if (!*cmd || *cmd == '#')
            continue;
        fprintf(stderr, "script %6u: %s\n", (unsigned)host_ms(), cmd);
        arg = cmd + strcspn(cmd, " \t");
        if (*arg)
            *arg++ = 0;
        if (!strcmp(cmd, "wait"))
            H.script_due = host_ms() + (uint32_t)atoi(arg);
        else if (!strcmp(cmd, "key")) {
            int shift = 0, ctrl = 0, alt = 0;
            SDL_Scancode sc;
            for (;;) {
                if (!SDL_strncasecmp(arg, "shift+", 6)) { shift = 1; arg += 6; }
                else if (!SDL_strncasecmp(arg, "ctrl+", 5)) { ctrl = 1; arg += 5; }
                else if (!SDL_strncasecmp(arg, "alt+", 4)) { alt = 1; arg += 4; }
                else break;
            }
            sc = SDL_GetScancodeFromName(arg);
            if (sc == SDL_SCANCODE_UNKNOWN)
                fprintf(stderr, "script: no key %s\n", arg);
            else
                script_key(sc, shift, ctrl, alt);
        } else if (!strcmp(cmd, "type")) {
            for (; *arg; arg++) {
                char c = *arg, name[2] = {0, 0};
                int shift = (c >= 'A' && c <= 'Z');
                SDL_Scancode sc;
                if (c == ' ')
                    sc = SDL_SCANCODE_SPACE;
                else if (c == '.')
                    sc = SDL_SCANCODE_PERIOD;
                else {
                    name[0] = c;
                    sc = SDL_GetScancodeFromName(name);
                }
                if (sc != SDL_SCANCODE_UNKNOWN)
                    script_key(sc, shift, 0, 0);
            }
        } else if (!strcmp(cmd, "move")) {
            int x = 0, y = 0;
            sscanf(arg, "%d %d", &x, &y);
            script_mouse(x, y, H.buttons);
        } else if (!strcmp(cmd, "down") || !strcmp(cmd, "up")) {
            int b = button_of(*arg ? arg : NULL);
            script_mouse(H.mouse_x, H.mouse_y, cmd[0] == 'd' ? H.buttons | b : H.buttons & ~b);
        } else if (!strcmp(cmd, "click")) {
            int x = 0, y = 0;
            char which[16] = "";
            sscanf(arg, "%d %d %15s", &x, &y, which);
            clicking = button_of(which[0] ? which : NULL);
            script_mouse(x, y, H.buttons);
            script_mouse(x, y, H.buttons | clicking);
            H.script_due = host_ms() + 50;
        } else if (!strcmp(cmd, "hold")) {
            H.script_held = (strstr(arg, "shift") ? HOST_SHIFT_LEFT : 0) | (strstr(arg, "ctrl") ? HOST_CTRL : 0)
                          | (strstr(arg, "alt") ? HOST_ALT : 0);
            H.shift = H.script_held;
        } else if (!strcmp(cmd, "shot"))
            write_bmp(arg);
        else if (!strcmp(cmd, "exit"))
            exit(0);
        else
            fprintf(stderr, "script: unknown command %s\n", cmd);
    }
}

void host_pump(void)
{
    SDL_Event e;
    script_step();
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_EVENT_QUIT:
            H.quit = 1;
            break;
        case SDL_EVENT_KEY_DOWN: {
            uint16_t code;
            const KeyDef *k = keydef(e.key.scancode);
            shift_from(e.key.mod);
            if (k && H.key_hook && H.key_hook(k->scan, 1, is_keypad(e.key.scancode)))
                break;
            code = host_key_translate(e.key.scancode, (e.key.mod & SDL_KMOD_SHIFT) != 0,
                                      (e.key.mod & SDL_KMOD_CTRL) != 0, (e.key.mod & SDL_KMOD_ALT) != 0);
            if (code)
                key_push(code);
            break;
        }
        case SDL_EVENT_KEY_UP: {
            const KeyDef *k = keydef(e.key.scancode);
            shift_from(e.key.mod);
            if (k && H.key_hook)
                H.key_hook(k->scan, 0, is_keypad(e.key.scancode));
            break;
        }
        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            float x, y;
            if (H.renderer)
                SDL_ConvertEventToRenderCoordinates(H.renderer, &e);
            if (e.type == SDL_EVENT_MOUSE_MOTION) {
                x = e.motion.x; y = e.motion.y;
            } else {
                int bit = e.button.button == SDL_BUTTON_LEFT ? HOST_BUTTON_LEFT
                        : e.button.button == SDL_BUTTON_RIGHT ? HOST_BUTTON_RIGHT : 0;
                if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) H.buttons |= bit;
                else H.buttons &= ~bit;
                x = e.button.x; y = e.button.y;
            }
            if (H.opt.aspect)
                y = y * HOST_H / 240;       /* the logical screen is 320x240 at 4:3 */
            H.mouse_x = x < 0 ? 0 : x >= HOST_W ? HOST_W - 1 : (int)x;
            H.mouse_y = y < 0 ? 0 : y >= HOST_H ? HOST_H - 1 : (int)y;
            if (H.mouse_handler)
                H.mouse_handler(H.mouse_x, H.mouse_y, H.buttons);
            break;
        }
        default:
            break;
        }
    }
    present_if_due();
}

int host_quit_requested(void) { return H.quit; }

/* the end of host_pump: a changed framebuffer is shown once per VGA frame (70 Hz) */
static void present_if_due(void)
{
    uint32_t now;
    if (!H.dirty)
        return;
    now = host_ms();
    if (now - H.last_present < 1000 / 70)
        return;
    H.dirty = 0;
    H.last_present = now;
    host_present();
}

void host_mouse(int *x, int *y, int *buttons)
{
    host_pump();
    if (x) *x = H.mouse_x;
    if (y) *y = H.mouse_y;
    if (buttons) *buttons = H.buttons;
}

void host_mouse_warp(int x, int y)
{
    H.mouse_x = x < 0 ? 0 : x >= HOST_W ? HOST_W - 1 : x;
    H.mouse_y = y < 0 ? 0 : y >= HOST_H ? HOST_H - 1 : y;
    if (H.window && H.renderer) {
        float wx, wy;
        float ly = ((float)H.mouse_y + 0.5f) * (H.opt.aspect ? 240.0f / HOST_H : 1.0f);
        SDL_RenderCoordinatesToWindow(H.renderer, (float)H.mouse_x + 0.5f, ly, &wx, &wy);
        SDL_WarpMouseInWindow(H.window, wx, wy);
    }
}

/* ---- video ------------------------------------------------------------------------------- */

uint8_t *host_vram(void) { return host_vga; }

void host_set_dac(int first, int count, const uint8_t *rgb6)
{
    int i;
    for (i = 0; i < count * 3 && first * 3 + i < (int)sizeof H.dac; i++)
        H.dac[first * 3 + i] = rgb6[i] & 0x3f;
}

void host_get_dac(int first, int count, uint8_t *rgb6)
{
    int i;
    for (i = 0; i < count * 3 && first * 3 + i < (int)sizeof H.dac; i++)
        rgb6[i] = H.dac[first * 3 + i];
}

const uint32_t *host_rgb(void) { return H.rgb; }

void host_present(void)
{
    uint32_t pal[256];
    int i;
    for (i = 0; i < 256; i++) {
        /* 6-bit DAC to 8 bits the way VGA monitors showed it: c << 2 | c >> 4 */
        uint32_t r = H.dac[i * 3], g = H.dac[i * 3 + 1], b = H.dac[i * 3 + 2];
        pal[i] = (r << 2 | r >> 4) << 16 | (g << 2 | g >> 4) << 8 | (b << 2 | b >> 4);
    }
    for (i = 0; i < HOST_W * HOST_H; i++)
        H.rgb[i] = pal[host_vga[i]];
    if (!H.renderer)
        return;
    SDL_UpdateTexture(H.texture, NULL, H.rgb, HOST_W * 4);
    SDL_SetRenderDrawColor(H.renderer, 0, 0, 0, 255);
    SDL_RenderClear(H.renderer);
    SDL_RenderTexture(H.renderer, H.texture, NULL, NULL);
    SDL_RenderPresent(H.renderer);
}

int host_vretrace(void)
{
    /* 70 frames a second; retrace is the last ~1.4 ms of each */
    uint32_t t = host_ms() * 70 % 1000;
    return t >= 900;
}

/* ---- time -------------------------------------------------------------------------------- */

uint32_t host_ms(void)
{
    if (H.virtual_time)
        return H.virtual_ms;
    return (uint32_t)((SDL_GetTicksNS() - H.t0) / 1000000u);
}

void host_virtual_time(int on) { H.virtual_ms = host_ms(); H.virtual_time = on; }
void host_advance(uint32_t ms) { H.virtual_ms += ms; }

void host_sleep(uint32_t ms)
{
    if (H.virtual_time)
        H.virtual_ms += ms;
    else
        SDL_Delay(ms);
}

/* ---- lifetime ---------------------------------------------------------------------------- */

int host_init(const HostOptions *opt)
{
    int scale, h;
    memset(&H, 0, sizeof H);
    H.opt = *opt;
    if (opt->headless)
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy");
    if (!SDL_Init(SDL_INIT_VIDEO))
        return -1;
    H.t0 = SDL_GetTicksNS();
    if (opt->script && !(H.script = fopen(opt->script, "r")))
        return -1;
    if (opt->headless)
        return 0;
    scale = opt->scale > 0 ? opt->scale : 3;
    h = opt->aspect ? 240 : HOST_H;
    if (!SDL_CreateWindowAndRenderer(opt->title ? opt->title : "Colonization Map Editor",
                                     HOST_W * scale, h * scale, SDL_WINDOW_RESIZABLE,
                                     &H.window, &H.renderer))
        return -1;
    /* the logical size is the 1994 screen: 320x200, or 320x240 with the picture stretched
       to 4:3; whole-number scales keep every pixel the same size */
    SDL_SetRenderLogicalPresentation(H.renderer, HOST_W, h, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
    H.texture = SDL_CreateTexture(H.renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING,
                                  HOST_W, HOST_H);
    if (!H.texture)
        return -1;
    SDL_SetTextureScaleMode(H.texture, SDL_SCALEMODE_NEAREST);
    SDL_HideCursor();                       /* the editor draws its own */
    return 0;
}

void host_quit(void)
{
    if (H.texture) SDL_DestroyTexture(H.texture);
    if (H.renderer) SDL_DestroyRenderer(H.renderer);
    if (H.window) SDL_DestroyWindow(H.window);
    SDL_Quit();
    memset(&H, 0, sizeof H);
}
