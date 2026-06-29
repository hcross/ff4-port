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
 *   5 / F5  save state to the next free incremental slot (<prefix>-NNN.lss,
 *           --save-prefix, default "seed"); numbering resumes across sessions.
 *           Prefer the digit keys: macOS grabs F5/F9/F12 unless Fn-locked.
 *   9 / F9  reload the most recent capture (or the --load seed if none)
 *   0 / F12 screenshot PPM (/tmp/ff4-desktop-shot.ppm)
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

/* Combat-graphics dispatch cluster (btlgfx bank $02 + DrawMP/ExecBtlGfx/
 * TfrSprites) — isolated by oracle bisection as the cause of the combat &
 * menu rendering glitches (all no_source / never spike-validated, ported as a
 * batch). Run them in the interpreter (correct ground truth on desktop) while
 * keeping the 162 proven routines native. DESKTOP-ONLY: on device TfrSprites
 * must stay native (F3 DMA hardfault). Returns 1 = run native, 0 = interpret. */
static int host_exclude_combatgfx(uint32_t pc) {
    switch (pc) {
        case 0x03fe03: case 0x028560: case 0x0285d2: case 0x0290a0:
        case 0x02a491: case 0x02bb0b: case 0x02bb1a: case 0x02da73:
        case 0x02dafe: case 0x02dced: case 0x02dda5: case 0x02dddc:
        case 0x03805f: case 0x038085:
            return 0;   /* proven-divergent → interpret */
        default:
            return 1;   /* everything else stays native */
    }
}

/* Broader triage filter: combat-graphics cluster PLUS the field-rendering
 * suspects for the mode-7 / tile glitches (motion-dependent, so A/B'd live
 * rather than via the no-input oracle). Toggle with 'm' while the field glitch
 * is on screen; if it clears, the culprit is in this set → narrow next. */
static int host_exclude_render(uint32_t pc) {
    if (host_exclude_combatgfx(pc) == 0) return 0;
    switch (pc) {
        /* mode-7 / scroll / HDMA */
        case 0x159104: case 0x1591ca: case 0x159204: case 0x15c144:
        case 0x15c163: case 0x15c23d: case 0x15ca5e: case 0x14fd0c:
        case 0x00f533: case 0x16f533:
        /* BG tile transfer / tilemap decode */
        case 0x15b143: case 0x16ffab: case 0x16fb93: case 0x00cb5f:
        case 0x1e9f6c:
            return 0;   /* field-render suspect → interpret */
        default:
            return 1;
    }
}

/* Default desktop filter: every PROVEN-divergent dispatched routine — the
 * combat-graphics cluster (bugs 1+2) plus TfrBGGfx ($15:B143, the tile-
 * corruption culprit isolated by bisection on 008-overworld-mode7, a DMA-from-C
 * routine that won't flush on the isolated desktop harness). Mode-7 (InitMapRAM)
 * was a REAL fix and stays native. */
static int host_exclude_divergent(uint32_t pc) {
    /* TfrBGGfx ($15:B143) is now a manual VRAM loop (no DMA-from-C) → runs
     * correctly native; no longer excluded. Mode-7 (InitMapRAM) also a real fix. */
    return host_exclude_combatgfx(pc);
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

/* First index N (1-based) for which "<prefix>-NNN.lss" does not yet exist, so
 * F5 never clobbers a previous capture and numbering resumes across sessions. */
static int next_free_index(const char *prefix) {
    char path[1024];
    for (int n = 1; n < 100000; n++) {
        snprintf(path, sizeof path, "%s-%03d.lss", prefix, n);
        FILE *f = fopen(path, "rb");
        if (!f) return n;
        fclose(f);
    }
    return 1;
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
        fprintf(stderr, "usage: %s <rom.sfc> [--load f.lss] [--save-prefix P] [--scale N]\n", argv[0]);
        return 2;
    }
    const char *rom_path = argv[1];
    const char *load_path = NULL;
    const char *save_prefix = "seed";   /* F5 -> <prefix>-NNN.lss, incremental */
    int scale = 2;
    for (int i = 2; i < argc; i++) {
        if      (!strcmp(argv[i], "--load")        && i+1 < argc) load_path   = argv[++i];
        else if (!strcmp(argv[i], "--save-prefix") && i+1 < argc) save_prefix = argv[++i];
        else if (!strcmp(argv[i], "--scale")       && i+1 < argc) scale = atoi(argv[++i]);
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
    /* Default ON: interpret the proven-divergent combat-graphics cluster so the
     * desktop renders combat/menu correctly while keeping the 162 proven
     * routines native. Toggle with 'b' to see the bug (all-native). */
    bool exclude_gfx = true;
    bool render_mode = false;   /* 'm' = broader field-render exclusion (mode-7/tiles) */
    ff4_dispatch_filter = host_exclude_divergent;
    uint64_t frame = 0;

    /* Incremental savestate slots: F5 writes the next free <prefix>-NNN.lss,
     * F9 reloads the most recent capture (or the --load seed if none yet). */
    int save_idx = next_free_index(save_prefix);
    char last_saved[1024]; bool have_saved = false;
    printf("[seeds] F5 -> %s-%03d.lss (incremental)\n", save_prefix, save_idx);

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
                    case SDLK_b: exclude_gfx = !exclude_gfx; interp_mode = false; render_mode = false;
                                 /* A/B the combat-graphics fix: ON = cluster interpreted
                                  * (correct), OFF = all-native (shows the rendering bug). */
                                 ff4_dispatch_filter = exclude_gfx ? host_exclude_divergent : NULL;
                                 printf("[combat-gfx] %s\n", exclude_gfx
                                        ? "EXCLUDED (interpreted — correct render)"
                                        : "native (bug visible)"); break;
                    case SDLK_m: render_mode = !render_mode; interp_mode = false;
                                 /* Broader field-render exclusion (mode-7/scroll/HDMA/tiles)
                                  * for live A/B of the mode-7 & tile glitches. */
                                 ff4_dispatch_filter = render_mode ? host_exclude_render
                                                                   : (exclude_gfx ? host_exclude_combatgfx : NULL);
                                 printf("[field-render] %s\n", render_mode
                                        ? "EXCLUDED (combat-gfx + mode7/scroll/tiles interpreted)"
                                        : "combat-gfx only"); break;
                    /* F5/F9/F12 are hijacked by macOS unless Fn-locked, so the
                     * digit aliases 5/9/0 are the reliable bindings. */
                    case SDLK_F5: case SDLK_5:  /* save to the next free incremental slot */
                                   snprintf(last_saved, sizeof last_saved, "%s-%03d.lss", save_prefix, save_idx);
                                   save_state(last_saved);
                                   have_saved = true; save_idx++; break;
                    case SDLK_F9: case SDLK_9:  /* reload most recent capture, else the --load seed */
                                   if (have_saved)      load_state(last_saved);
                                   else if (load_path)  load_state(load_path);
                                   else printf("[state] nothing saved yet (press 5/F5)\n"); break;
                    case SDLK_F12: case SDLK_0: screenshot_ppm("/tmp/ff4-desktop-shot.ppm", fb); break;
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
