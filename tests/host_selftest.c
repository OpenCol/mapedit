/* host_selftest.c -- the host layer, headless: the DAC and framebuffer, the BIOS key codes,
 * the keyboard queue and mouse state from injected events, and virtual time. */
#include "host.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(cond, ...) do { if (!(cond)) { failures++; printf("FAIL %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); printf("\n"); } } while (0)

static void push_key(SDL_Scancode sc, SDL_Keymod mod)
{
    SDL_Event e;
    memset(&e, 0, sizeof e);
    e.type = SDL_EVENT_KEY_DOWN;
    e.key.scancode = sc;
    e.key.mod = mod;
    e.key.down = true;
    SDL_PushEvent(&e);
}

static void push_button(int x, int y, Uint8 button, int down)
{
    SDL_Event e;
    memset(&e, 0, sizeof e);
    e.type = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
    e.button.button = button;
    e.button.down = down;
    e.button.x = (float)x;
    e.button.y = (float)y;
    SDL_PushEvent(&e);
}

int main(void)
{
    HostOptions opt;
    uint8_t dac[6] = {63, 0, 0, 0, 32, 63}, back[6];
    int x, y, b;

    memset(&opt, 0, sizeof opt);
    opt.headless = 1;
    if (host_init(&opt) != 0) {
        printf("FAIL host_init: %s\n", SDL_GetError());
        return 1;
    }

    /* the DAC keeps 6 bits, and presents them as VGA did: c << 2 | c >> 4 */
    host_set_dac(1, 2, dac);
    host_get_dac(1, 2, back);
    CHECK(memcmp(dac, back, 6) == 0, "DAC round trip");
    host_vram()[0] = 1;
    host_vram()[HOST_W * HOST_H - 1] = 2;
    host_present();
    CHECK(host_rgb()[0] == 0xff0000, "index 1 -> %06x", host_rgb()[0]);
    CHECK(host_rgb()[HOST_W * HOST_H - 1] == 0x0082ff, "index 2 -> %06x", host_rgb()[HOST_W * HOST_H - 1]);

    /* BIOS INT 16h codes */
    CHECK(host_key_translate(SDL_SCANCODE_A, 0, 0, 0) == 0x1e61, "a");
    CHECK(host_key_translate(SDL_SCANCODE_A, 1, 0, 0) == 0x1e41, "A");
    CHECK(host_key_translate(SDL_SCANCODE_A, 0, 1, 0) == 0x1e01, "ctrl-a");
    CHECK(host_key_translate(SDL_SCANCODE_A, 0, 0, 1) == 0x1e00, "alt-a");
    CHECK(host_key_translate(SDL_SCANCODE_ESCAPE, 0, 0, 0) == 0x011b, "esc");
    CHECK(host_key_translate(SDL_SCANCODE_RETURN, 0, 0, 0) == 0x1c0d, "enter");
    CHECK(host_key_translate(SDL_SCANCODE_BACKSPACE, 0, 0, 0) == 0x0e08, "backspace");
    CHECK(host_key_translate(SDL_SCANCODE_SPACE, 0, 0, 0) == 0x3920, "space");
    CHECK(host_key_translate(SDL_SCANCODE_UP, 0, 0, 0) == 0x4800, "up");
    CHECK(host_key_translate(SDL_SCANCODE_KP_8, 0, 0, 0) == 0x4800, "keypad up");
    CHECK(host_key_translate(SDL_SCANCODE_F1, 0, 0, 0) == 0x3b00, "F1");
    CHECK(host_key_translate(SDL_SCANCODE_F1, 1, 0, 0) == 0x5400, "shift-F1");
    CHECK(host_key_translate(SDL_SCANCODE_F10, 0, 0, 1) == 0x7100, "alt-F10");
    CHECK(host_key_translate(SDL_SCANCODE_1, 0, 0, 1) == 0x7800, "alt-1");
    CHECK(host_key_translate(SDL_SCANCODE_LSHIFT, 1, 0, 0) == 0, "shift alone gives no code");

    /* the queue, from SDL events */
    host_key_flush();
    push_key(SDL_SCANCODE_Z, SDL_KMOD_NONE);
    push_key(SDL_SCANCODE_X, SDL_KMOD_LSHIFT);
    CHECK(host_key_ready(), "key ready");
    CHECK(host_key_get() == 0x2c7a, "z first");
    CHECK(host_shift_flags() == HOST_SHIFT_LEFT, "left shift held");
    CHECK(host_key_get() == 0x2d58, "then X");
    CHECK(!host_key_ready(), "queue empty");

    /* the mouse: headless, positions are screen pixels */
    push_button(100, 50, SDL_BUTTON_LEFT, 1);
    host_mouse(&x, &y, &b);
    CHECK(x == 100 && y == 50 && b == HOST_BUTTON_LEFT, "press at %d,%d buttons %d", x, y, b);
    push_button(400, -5, SDL_BUTTON_LEFT, 0);
    host_mouse(&x, &y, &b);
    CHECK(x == HOST_W - 1 && y == 0 && b == 0, "release clamped at %d,%d buttons %d", x, y, b);

    /* virtual time moves only when told */
    host_virtual_time(1);
    {
        uint32_t t = host_ms();
        host_sleep(55);
        CHECK(host_ms() == t + 55, "sleep advances virtual time");
        host_advance(1000);
        CHECK(host_ms() == t + 1055, "advance");
    }

    host_quit();
    printf("%s (%d failure%s)\n", failures ? "FAILED" : "host self-test passed", failures, failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
