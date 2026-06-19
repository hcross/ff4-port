/* FF4 desktop validation host — M2b interactive SDL frontend.
 *
 * See docs/adr/0001-desktop-validation-in-re-loop.md. Drives the live ff4-gnw
 * working tree through the same ff4_* glue the G&W device uses, with a window
 * so scenes can be reached by hand and trustworthy savestate seeds produced
 * for the M3 A/B oracle.
 *
 * Controls:
 *   arrows  d-pad        z B   x A   a Y   s X   d L   c R
 *   RShift  Select       Return Start
 *   Space   pause        .      single-frame step
 *   g       toggle interpreter mode (live A/B). Interpreter mode runs every
 *           FAITHFUL routine through the interpreter but keeps the host-
 *           critical reimplementations native (input + sound) so the host
 *           stays drivable — see host_keep_native().
 *   F5      save state   F9     load state   (slot: --state, default below)
 *   F12     screenshot PPM (/tmp/ff4-desktop-shot.ppm)
 *   Esc     quit
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>

#include "snes/snes.h"

extern bool ff4_init(const uint8_t *rom_bytes, int rom_length);
extern void ff4_step(void);
extern void ff4_shutdown(void);
extern void ff4_blit_to_lcd(uint16_t *lcd_fb);
extern void ff4_set_button(int player, int button, bool pressed);
extern Snes *ff4_snes;
extern uint32_t ff4_dispatch_hits;
extern uint32_t ff4_dispatch_misses;
extern int ff4_dispatch_enabled;
extern int (*ff4_dispatch_filter)(uint32_t pc);

/* 'g' interpreter mode keeps the host-critical reimplemented routines native
 * and interprets everything else — a global dispatch-off would break the host:
 *   018010 UpdateCtrlField_ext, 14fd03 UpdateCtrl_ext, 14fd00 InitCtrl_ext2 —
 *     read portAutoRead and rebuild the WRAM joypad mirror; the original asm
 *     input path is incompatible with the auto-joypad-enabled harness, so
 *     interpreting them kills all controller input (verified via input_probe).
 *   048004 ExecSound_ext_stub — bypasses the SPC sound wait; interpreting the
 *     real routine can stall the title.
 * NOT kept native: 15cadc (OAM-DMA bypass) — its real DMA works on desktop, so
 * letting it interpret shows ground-truth sprite rendering, which is the point. */
static int host_keep_native(uint32_t pc) {
    return pc == 0x018010 || pc == 0x14fd03 || pc == 0x14fd00 || pc == 0x048004;
}

#define LCD_W 320
#define LCD_H 240

static uint8_t *read_file(const char *path, long *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    if (n <= 0) { fclose(f); return NULL; }
    uint8_t *buf = malloc((size_t)n);
    if (!buf || fread(buf, 1, (size_t)n, f) != (size_t)n) { free(buf); fclose(f); return NULL; }
    fclose(f); *out_len = n; return buf;
}

/* LakeSnes button index for an SDL keycode, or -1. */
static int key_to_button(SDL_Keycode k) {
    switch (k) {
        case SDLK_z: return 0;  /* B */
        case SDLK_a: return 1;  /* Y */
        case SDLK_RSHIFT: return 2;  /* Select */
        case SDLK_RETURN: return 3;  /* Start */
        case SDLK_UP: return 4;
        case SDLK_DOWN: return 5;
        case SDLK_LEFT: return 6;
        case SDLK_RIGHT: return 7;
        case SDLK_x: return 8;  /* A */
        case SDLK_s: return 9;  /* X */
        case SDLK_d: return 10; /* L */
        case SDLK_c: return 11; /* R */
        default: return -1;
    }
}

static void save_state(const char *path) {
    int sz = snes_saveState(ff4_snes, NULL);
    uint8_t *buf = malloc((size_t)sz);
    if (!buf) return;
    snes_saveState(ff4_snes, buf);
    FILE *o = fopen(path, "wb");
    if (o) { fwrite(buf, (size_t)sz, 1, o); fclose(o); printf("[state] saved %s (%d bytes)\n", path, sz); }
    else printf("[state] cannot write %s\n", path);
    free(buf);
}

static void load_state(const char *path) {
    long n = 0; uint8_t *st = read_file(path, &n);
    if (!st) { printf("[state] cannot read %s\n", path); return; }
    bool ok = snes_loadState(ff4_snes, st, (int)n);
    free(st);
    printf("[state] load %s: %s | pc=%02X:%04X\n", path, ok ? "ok" : "REJECTED",
           ff4_snes->cpu->k, ff4_snes->cpu->pc);
}

static void screenshot_ppm(const char *path, const uint16_t *fb) {
    FILE *o = fopen(path, "wb");
    if (!o) return;
    fprintf(o, "P6\n%d %d\n255\n", LCD_W, LCD_H);
    for (int i = 0; i < LCD_W * LCD_H; i++) {
        uint16_t px = fb[i];
        uint8_t r5 = (px >> 11) & 0x1F, g6 = (px >> 5) & 0x3F, b5 = px & 0x1F;
        uint8_t rgb[3] = { (uint8_t)((r5<<3)|(r5>>2)), (uint8_t)((g6<<2)|(g6>>4)), (uint8_t)((b5<<3)|(b5>>2)) };
        fwrite(rgb, 1, 3, o);
    }
    fclose(o);
    printf("[shot] %s\n", path);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <rom.sfc> [--load f.lss] [--state f.lss] [--scale N]\n", argv[0]);
        return 2;
    }
    const char *rom_path = argv[1];
    const char *load_path = NULL;
    const char *state_path = "/tmp/ff4-desktop.lss";
    int scale = 2;
    for (int i = 2; i < argc; i++) {
        if      (!strcmp(argv[i], "--load")  && i+1 < argc) load_path  = argv[++i];
        else if (!strcmp(argv[i], "--state") && i+1 < argc) state_path = argv[++i];
        else if (!strcmp(argv[i], "--scale") && i+1 < argc) scale = atoi(argv[++i]);
        else { fprintf(stderr, "error: bad arg '%s'\n", argv[i]); return 2; }
    }

    long rom_len = 0;
    uint8_t *rom = read_file(rom_path, &rom_len);
    if (!rom) { fprintf(stderr, "error: cannot read ROM '%s'\n", rom_path); return 1; }
    if (!ff4_init(rom, (int)rom_len)) { fprintf(stderr, "error: ff4_init failed\n"); return 1; }
    if (load_path) load_state(load_path);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) { fprintf(stderr, "SDL_Init: %s\n", SDL_GetError()); return 1; }
    SDL_Window *win = SDL_CreateWindow("FF4 desktop (ff4-gnw)", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, LCD_W * scale, LCD_H * scale, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetLogicalSize(ren, LCD_W, LCD_H);
    SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, LCD_W, LCD_H);

    static uint16_t fb[LCD_W * LCD_H];
    bool running = true, paused = false, interp_mode = false;
    uint64_t frame = 0;

    while (running) {
        SDL_Event e;
        bool step_once = false;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
                bool down = (e.type == SDL_KEYDOWN);
                int btn = key_to_button(e.key.keysym.sym);
                if (btn >= 0) { ff4_set_button(1, btn, down); continue; }
                if (!down) continue;
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE: running = false; break;
                    case SDLK_SPACE:  paused = !paused; printf("[%s]\n", paused ? "paused" : "running"); break;
                    case SDLK_PERIOD: step_once = true; break;
                    case SDLK_g: interp_mode = !interp_mode;
                                 /* Dispatch stays on; the filter chooses per hook. In interpreter
                                  * mode every faithful routine falls through to the interpreter,
                                  * but the host-critical reimplementations stay native. */
                                 ff4_dispatch_filter = interp_mode ? host_keep_native : NULL;
                                 printf("[dispatch] %s\n", interp_mode
                                        ? "interpreter (input+sound kept native)" : "NATIVE"); break;
                    case SDLK_F5:  save_state(state_path); break;
                    case SDLK_F9:  load_state(state_path); break;
                    case SDLK_F12: screenshot_ppm("/tmp/ff4-desktop-shot.ppm", fb); break;
                    default: break;
                }
            }
        }

        if (!paused || step_once) { ff4_step(); frame++; }

        ff4_blit_to_lcd(fb);
        SDL_UpdateTexture(tex, NULL, fb, LCD_W * sizeof(uint16_t));
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, NULL, NULL);
        SDL_RenderPresent(ren);

        if ((frame & 63) == 0) {
            uint32_t tot = ff4_dispatch_hits + ff4_dispatch_misses;
            char title[160];
            snprintf(title, sizeof title,
                "FF4 desktop | pc=%02X:%04X | dispatch %s %.0f%% | frame %llu%s",
                ff4_snes->cpu->k, ff4_snes->cpu->pc,
                interp_mode ? "INTERP" : "NATIVE",
                tot ? 100.0 * ff4_dispatch_hits / tot : 0.0,
                (unsigned long long)frame, paused ? " [paused]" : "");
            SDL_SetWindowTitle(win, title);
        }
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    ff4_shutdown();
    free(rom);
    return 0;
}
