/* FF4 desktop validation host — headless harness (M0/M1/M2a).
 *
 * See docs/adr/0001-desktop-validation-in-re-loop.md. Runs the live ff4-gnw
 * working tree on desktop through the same ff4_* glue the G&W device uses.
 *
 * Usage:
 *   ff4-desktop-headless <rom.sfc> [flags]
 *     --frames N        run N frames (default 600)
 *     --budget OPS      CPU-opcode budget per frame (default 4000000); exits 3
 *                       instead of hanging forever if a frame never reaches vblank
 *     --load  <f.lss>   load a LakeSnes savestate after init (jump to a scene)
 *     --save  <f.lss>   save a savestate after the run (reusable seed)
 *     --out   <f.ppm>   dump the final frame as binary PPM
 *
 * Exit codes: 0=completed all frames, 1=I/O or init error, 2=usage,
 * 3=stalled (opcode budget exhausted inside one frame — a hang).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <stdbool.h>

#include "snes/snes.h"   /* Snes, snes_loadState/saveState */
#include "snes/ppu.h"    /* ppu->vramGen (--vramgen-delta diagnostics) */

/* ff4-gnw glue (ff4-gnw/main.c) */
extern bool ff4_init(const uint8_t *rom_bytes, int rom_length);
extern void ff4_step(void);
extern void init_ctrl_emu(Snes *snes);
extern void ff4_shutdown(void);
extern void ff4_blit_to_lcd(uint16_t *lcd_fb);
extern void ff4_set_button(int player, int button, bool pressed);
extern Snes *ff4_snes;
/* Weak DEFINITIONS, not externs: let this harness also link against older
 * ff4-gnw checkouts that predate these hooks (A/B archaeology builds) --
 * the ff4-gnw strong definitions win whenever they exist. On a pre-hook
 * checkout the affected flags (--render-every, --watch-wram) silently
 * degrade to no-ops, which is fine for archaeology runs. */
int ff4_ppu_render_enabled __attribute__((weak)) = 1;            /* snes/ppu.c  */
void (*snes_wram_write_hook)(uint32_t, uint8_t, void *)
    __attribute__((weak)) = 0;                                   /* snes/snes.c */
void *snes_wram_write_hook_ctx __attribute__((weak)) = 0;        /* snes/snes.c */

/* --interp-except-input: mirrors main_sdl.c's 'g' key exactly (host_keep_native)
 * -- interpret everything EXCEPT the input-mirror writers and the field-menu
 * text-window primitive, which stay native. Needed to headlessly reproduce an
 * SDL repro captured with 'g' toggled on: --no-dispatch disables ALL dispatch
 * including the input hooks, which is a DIFFERENT (already-documented, separate)
 * "input dead in interpreter mode" bug -- not what 'g' actually does. */
static int host_keep_native(uint32_t pc) {
    return pc == 0x018010 || pc == 0x14fd03 || pc == 0x14fd00 || pc == 0x048004;
}

/* --press BUTTON:FRAME[:HOLD] (repeatable) — inject a scripted button press
 * at a given 1-based frame number, held for HOLD frames (default 1). Button
 * indices match main_sdl.c's key_to_button (same LakeSnes controller layout):
 * B=0 Y=1 Select=2 Start=3 Up=4 Down=5 Left=6 Right=7 A=8 X=9 L=10 R=11. Lets
 * a headless run reproduce an SDL repro (e.g. "open the menu") without a
 * human at the keyboard. */
#define PRESS_MAX 64
static int      g_press_btn[PRESS_MAX];
static int      g_press_frame[PRESS_MAX];
static int      g_press_hold[PRESS_MAX];
static int      g_press_n = 0;

static int button_name_to_index(const char *name) {
    static const char *names[12] = {
        "b", "y", "select", "start", "up", "down", "left", "right",
        "a", "x", "l", "r"
    };
    for (int i = 0; i < 12; i++)
        if (!strcasecmp(name, names[i])) return i;
    return -1;
}

static int parse_press_spec(const char *spec) {
    if (g_press_n >= PRESS_MAX) return 0;
    char buf[64];
    strncpy(buf, spec, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    char *btn_s = strtok(buf, ":");
    char *frame_s = btn_s ? strtok(NULL, ":") : NULL;
    char *hold_s = frame_s ? strtok(NULL, ":") : NULL;
    if (!btn_s || !frame_s) return 0;
    int btn = button_name_to_index(btn_s);
    if (btn < 0) return 0;
    g_press_btn[g_press_n]   = btn;
    g_press_frame[g_press_n] = atoi(frame_s);
    g_press_hold[g_press_n]  = hold_s ? atoi(hold_s) : 1;
    g_press_n++;
    return 1;
}

unsigned ff4_diag_trc_miss; /* R2b tile-row-cache miss counter (ppu.c diag) */

/* dispatch counters + runtime toggle (ff4-gnw/dispatch_all.c) */
extern uint32_t ff4_dispatch_hits;
extern uint32_t ff4_dispatch_misses;
extern int ff4_dispatch_enabled;
extern void (*ff4_dispatch_trace)(uint32_t pc);
extern int  (*ff4_dispatch_filter)(uint32_t pc);

static int trace_frame = -1;
static int trace_all = 0;
static int trace_cur_frame = 0;
static void dispatch_trace_cb(uint32_t pc) {
    printf("  hit: %06X frame=%d\n", pc, trace_cur_frame);
}

/* --exclude PC (repeatable): force the listed routines to pure interpretation
 * (filter returns 0) while everything else stays native. Lets a headless FB
 * dump A/B a suspect cluster against ground truth without the oracle's
 * first-divergence cutoff. */
#define EXCL_MAX 32
static uint32_t g_excl[EXCL_MAX]; static int g_excl_n = 0;
static int excl_filter(uint32_t pc) {
    for (int i = 0; i < g_excl_n; i++) if (g_excl[i] == pc) return 0;
    return 1;
}

/* WRAM watchpoint — logs every write to a given WRAM offset with SNES PC. */
static uint32_t watch_wram_addr = (uint32_t)-1;
static int       watch_cur_frame = 0;

static uint32_t watch_wram_hi = 0;  /* inclusive upper bound for range watch; 0 = exact */

static void wram_watch_cb(uint32_t wram_off, uint8_t val, void *ctx) {
    uint32_t hi = watch_wram_hi ? watch_wram_hi : watch_wram_addr;
    if (wram_off < watch_wram_addr || wram_off > hi) return;
    Snes *s = (Snes *)ctx;
    printf("  [watch $%05X] frame=%-4d val=0x%02X  PC=%02X:%04X\n",
           wram_off, watch_cur_frame, val, s->cpu->k, s->cpu->pc);
}

#define LCD_W 320
#define LCD_H 240

static uint32_t crc32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++)
            crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)(-(int32_t)(crc & 1)));
    }
    return ~crc;
}

/* Read a whole file into a malloc'd buffer; *out_len set on success. */
static uint8_t *read_file(const char *path, long *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n <= 0) { fclose(f); return NULL; }
    uint8_t *buf = malloc((size_t)n);
    if (!buf || fread(buf, 1, (size_t)n, f) != (size_t)n) { free(buf); fclose(f); return NULL; }
    fclose(f);
    *out_len = n;
    return buf;
}

/* Dump the current frame (ff4_blit_to_lcd RGB565 → RGB888) as a binary PPM. */
static int dump_ppm(const char *path) {
    static uint16_t fb[LCD_W * LCD_H];
    for (int i = 0; i < LCD_W * LCD_H; i++) fb[i] = 0;
    ff4_blit_to_lcd(fb);
    FILE *o = fopen(path, "wb");
    if (!o) { fprintf(stderr, "error: cannot write '%s'\n", path); return 0; }
    fprintf(o, "P6\n%d %d\n255\n", LCD_W, LCD_H);
    for (int i = 0; i < LCD_W * LCD_H; i++) {
        uint16_t px = fb[i];
        uint8_t r5 = (px >> 11) & 0x1F, g6 = (px >> 5) & 0x3F, b5 = px & 0x1F;
        uint8_t rgb[3] = {
            (uint8_t)((r5 << 3) | (r5 >> 2)),
            (uint8_t)((g6 << 2) | (g6 >> 4)),
            (uint8_t)((b5 << 3) | (b5 >> 2)),
        };
        fwrite(rgb, 1, 3, o);
    }
    fclose(o);
    return 1;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,
            "usage: %s <rom.sfc> [--frames N] [--budget OPS] [--load f.lss] [--save f.lss] [--out f.ppm]\n"
            "       [--press BUTTON:FRAME[:HOLD]] (repeatable; BUTTON=start|select|a|b|x|y|l|r|up|down|left|right)\n",
            argv[0]);
        return 2;
    }
    const char *rom_path = argv[1];
    int frames = 600;
    uint64_t budget = 4000000;   /* CPU-opcode budget per frame; matches oracle_ab.c's
                                  * default so a hang trips the same signal here */
    const char *load_path = NULL, *save_path = NULL, *out_ppm = NULL;
    const char *dump_wram_path = NULL;
    int force_init_ctrl = 0;
    int render_every = 1;   /* --render-every K: frameskip validation -- render
                             * only frames where (frame % K) == K-1, mirroring
                             * the device loop's emulate-N-render-1 pattern.
                             * WRAM/emulation state must be identical to K=1
                             * by construction (ppu_runLine only skips the
                             * pixel loop; sprite evaluation still runs). */
    int audio_crc = 0;   /* --audio-crc: per-frame CRC of the DSP output (APU evidence) */
    int walk_square_start = 0;  /* --walk-square START: from frame START on,
                             * loop the device's FF4_AUTO_WALK square (30
                             * frames per DPAD direction) forever. 0 = off. */
    int vramgen_delta = 0;  /* --vramgen-delta: print the per-frame VRAM-write
                             * generation delta (ppu->vramGen). Any non-zero
                             * frame invalidates the WHOLE R2b decoded-tile-row
                             * cache, so this is the direct evidence channel
                             * for cache-invalidation hypotheses (idle vs
                             * walking scroll workloads). */
    int fb_crc = 0;         /* --fb-crc: print the blitted framebuffer's crc32
                             * EVERY frame. Purpose: transient-artifact hunts --
                             * the final-frame PPM proves nothing about frames
                             * in between; diffing per-frame CRC logs between
                             * two builds pins the exact frames where their
                             * rendering diverges (found 2026-07-09 while
                             * chasing a fleeting battle-menu artifact). */
    int out_every = 0;      /* --out-every N PREFIX: dump PREFIX%04d.ppm every
                             * N frames -- exhaustive visual sweep companion to
                             * --fb-crc for the same transient-artifact hunts. */
    const char *out_every_prefix = NULL;

    for (int i = 2; i < argc; i++) {
        if      (!strcmp(argv[i], "--frames") && i + 1 < argc) frames = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--budget") && i + 1 < argc) budget = strtoull(argv[++i], NULL, 0);
        else if (!strcmp(argv[i], "--load")   && i + 1 < argc) load_path = argv[++i];
        else if (!strcmp(argv[i], "--save")   && i + 1 < argc) save_path = argv[++i];
        else if (!strcmp(argv[i], "--dump-wram") && i + 1 < argc) dump_wram_path = argv[++i];
        else if (!strcmp(argv[i], "--out")    && i + 1 < argc) out_ppm = argv[++i];
        else if (!strcmp(argv[i], "--no-dispatch")) ff4_dispatch_enabled = 0;
        else if (!strcmp(argv[i], "--trace-frame") && i + 1 < argc) trace_frame = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--watch-wram") && i + 1 < argc) watch_wram_addr = (uint32_t)strtoul(argv[++i], NULL, 16);
        else if (!strcmp(argv[i], "--watch-wram-hi") && i + 1 < argc) watch_wram_hi = (uint32_t)strtoul(argv[++i], NULL, 16);
        else if (!strcmp(argv[i], "--exclude") && i + 1 < argc && g_excl_n < EXCL_MAX) g_excl[g_excl_n++] = (uint32_t)strtoul(argv[++i], NULL, 16);
        else if (!strcmp(argv[i], "--press") && i + 1 < argc) {
            if (!parse_press_spec(argv[++i])) { fprintf(stderr, "error: bad --press spec '%s' (want BUTTON:FRAME[:HOLD])\n", argv[i]); return 2; }
        }
        else if (!strcmp(argv[i], "--interp-except-input")) ff4_dispatch_filter = host_keep_native;
        else if (!strcmp(argv[i], "--force-init-ctrl")) force_init_ctrl = 1;
        else if (!strcmp(argv[i], "--trace-all")) trace_all = 1;
        else if (!strcmp(argv[i], "--render-every") && i + 1 < argc) {
            render_every = atoi(argv[++i]);
            if (render_every < 1) { fprintf(stderr, "error: --render-every wants K >= 1\n"); return 2; }
        }
        else if (!strcmp(argv[i], "--fb-crc")) fb_crc = 1;
        else if (!strcmp(argv[i], "--audio-crc")) audio_crc = 1;
        else if (!strcmp(argv[i], "--vramgen-delta")) vramgen_delta = 1;
        else if (!strcmp(argv[i], "--walk-square") && i + 1 < argc) walk_square_start = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--out-every") && i + 2 < argc) {
            out_every = atoi(argv[++i]);
            out_every_prefix = argv[++i];
            if (out_every < 1) { fprintf(stderr, "error: --out-every wants N >= 1 and a path prefix\n"); return 2; }
        }
        else { fprintf(stderr, "error: bad arg '%s'\n", argv[i]); return 2; }
    }
    if (g_excl_n > 0) ff4_dispatch_filter = excl_filter;

    long rom_len = 0;
    uint8_t *rom = read_file(rom_path, &rom_len);
    if (!rom) { fprintf(stderr, "error: cannot read ROM '%s'\n", rom_path); return 1; }

    if (!ff4_init(rom, (int)rom_len)) {
        fprintf(stderr, "error: ff4_init failed\n"); free(rom); return 1;
    }
    printf("ff4_init ok: rom=%ld bytes\n", rom_len);

    if (load_path) {
        long st_len = 0;
        uint8_t *st = read_file(load_path, &st_len);
        if (!st) { fprintf(stderr, "error: cannot read state '%s'\n", load_path); free(rom); return 1; }
        bool ok = snes_loadState(ff4_snes, st, (int)st_len);
        free(st);
        if (!ok) { fprintf(stderr, "error: snes_loadState rejected '%s' (%ld bytes)\n", load_path, st_len); free(rom); return 1; }
        printf("loaded state  : %s (%ld bytes) | pc=%02X:%04X\n",
               load_path, st_len, ff4_snes->cpu->k, ff4_snes->cpu->pc);
        printf("  ram[0x1A05..0x1A1C] (ctrl remap table): ");
        for (int i = 0; i < 24; i++) printf("%02X ", ff4_snes->ram[0x1A05 + i]);
        printf("\n");
        if (force_init_ctrl) {
            /* Testing aid only: pre-2026-07-06 savestates were captured
             * under a no-op InitCtrl stub, so their button-remap table
             * ($1A05+) is baked in as all-zero -- a real fresh boot after
             * the fix populates it correctly, but this lets an OLD
             * savestate exercise the fixed ReadCtrl/UpdateCtrl algorithm
             * for verification without a full recapture. */
            init_ctrl_emu(ff4_snes);
            printf("  [force-init-ctrl] ram[0x1A05..0x1A1C] now: ");
            for (int i = 0; i < 24; i++) printf("%02X ", ff4_snes->ram[0x1A05 + i]);
            printf("\n");
        }
    }

    if (watch_wram_addr != (uint32_t)-1) {
        snes_wram_write_hook     = wram_watch_cb;
        snes_wram_write_hook_ctx = ff4_snes;
        printf("watching WRAM $%05X for writes\n", watch_wram_addr);
    }

    printf("running %d frames (budget %llu ops/frame)...\n", frames, (unsigned long long)budget);
    bool stalled = false;
    for (int i = 0; i < frames; i++) {
        watch_cur_frame = i + 1;
        if (walk_square_start && i + 1 >= walk_square_start) {
            /* Desktop mirror of the device's FF4_AUTO_WALK: DPAD square,
             * 30 frames per side (120-frame period), looping forever. Keeps
             * the map scrolling every frame so the R4/R5 render skips never
             * fire -- the same "real play" workload the D6R ring measures. */
            static const int walk_btn[4] = { 4 /*up*/, 7 /*right*/, 5 /*down*/, 6 /*left*/ };
            const int t = (i + 1 - walk_square_start) % 120;
            const int phase = t / 30;
            if (t % 30 == 0) {
                ff4_set_button(1, walk_btn[(phase + 3) & 3], false);
                ff4_set_button(1, walk_btn[phase], true);
            }
        }
        for (int p = 0; p < g_press_n; p++) {
            int frame_1based = i + 1;
            if (frame_1based == g_press_frame[p]) {
                ff4_set_button(1, g_press_btn[p], true);
                printf("  [press] frame=%-4d btn=%d DOWN\n", frame_1based, g_press_btn[p]);
            }
            if (frame_1based == g_press_frame[p] + g_press_hold[p]) {
                ff4_set_button(1, g_press_btn[p], false);
                printf("  [press] frame=%-4d btn=%d UP\n", frame_1based, g_press_btn[p]);
            }
        }
        trace_cur_frame = i + 1;
        if (trace_all)
            ff4_dispatch_trace = dispatch_trace_cb;
        else if (trace_frame >= 0 && i == trace_frame - 1)
            ff4_dispatch_trace = dispatch_trace_cb;
        else if (trace_frame >= 0 && i == trace_frame)
            ff4_dispatch_trace = NULL;
        ff4_ppu_render_enabled = (render_every == 1) || (i % render_every == render_every - 1);
        if (!snes_runFrameBounded(ff4_snes, budget)) {
            fprintf(stderr, "error: stalled at frame %d (opcode budget %llu exhausted — hang)\n",
                    i + 1, (unsigned long long)budget);
            stalled = true;
            break;
        }
        if ((i + 1) % 60 == 0)
            printf("  frame %4d | pc=%02X:%04X | hits=%u misses=%u\n",
                   i + 1, ff4_snes->cpu->k, ff4_snes->cpu->pc,
                   ff4_dispatch_hits, ff4_dispatch_misses);
        { extern unsigned ff4_diag_trc_miss; static unsigned prev_trc;
          if (vramgen_delta) { printf("TRCMISS %d %u\n", i + 1, ff4_diag_trc_miss - prev_trc); prev_trc = ff4_diag_trc_miss; } }
        if (vramgen_delta) {
            static uint32_t vg_prev = 0;
            static int      vg_primed = 0;
            uint32_t vg = ff4_snes->ppu->vramGen;
            printf("VRAMGEN %d %u\n", i + 1, vg_primed ? vg - vg_prev : 0);
            vg_prev = vg; vg_primed = 1;
        }
        if (audio_crc) {
            /* Pull one frame of DSP output through the same public API the
             * device uses (catchup + resample) and CRC it: the byte-exact
             * audio evidence channel for APU/DSP optimizations -- FB/WRAM
             * CRCs are structurally deaf to the sound path. 534 stereo
             * samples = the native NTSC per-frame count (no resampling
             * ambiguity). */
            static int16_t aud[534 * 2];
            memset(aud, 0, sizeof(aud));
            snes_setSamples(ff4_snes, aud, 534);
            printf("AUDCRC %d %08X\n", i + 1, crc32((const uint8_t *)aud, sizeof(aud)));
        }
        if (fb_crc) {
            static uint16_t fbc[320 * 240];
            memset(fbc, 0, sizeof(fbc));
            ff4_blit_to_lcd(fbc);
            printf("FBCRC %d %08X\n", i + 1, crc32((const uint8_t *)fbc, sizeof(fbc)));
        }
        if (out_every && ((i + 1) % out_every) == 0) {
            char path[1024];
            snprintf(path, sizeof(path), "%s%04d.ppm", out_every_prefix, i + 1);
            dump_ppm(path);
        }
    }

    uint32_t wram_crc = crc32(ff4_snes->ram, sizeof(ff4_snes->ram));
    uint32_t total = ff4_dispatch_hits + ff4_dispatch_misses;
    double hit_rate = total ? (100.0 * ff4_dispatch_hits / total) : 0.0;

    printf("\n=== result ===\n");
    printf("frames run     : %u\n", ff4_snes->frames);
    printf("cpu cycles     : %llu\n", (unsigned long long)ff4_snes->cycles);
    printf("dispatch       : %.1f%% (%u/%u)\n", hit_rate, ff4_dispatch_hits, total);
    printf("WRAM crc32     : %08X\n", wram_crc);
    printf("stalled        : %s\n", stalled ? "yes (opcode budget exhausted — hang)" : "no");

    if (out_ppm && dump_ppm(out_ppm))
        printf("frame dumped   : %s (%dx%d)\n", out_ppm, LCD_W, LCD_H);

    if (dump_wram_path) {
        FILE *o = fopen(dump_wram_path, "wb");
        if (o) { fwrite(ff4_snes->ram, 1, 0x20000, o); fclose(o);
                 printf("wram dumped    : %s (131072 bytes)\n", dump_wram_path); }
    }

    if (save_path) {
        int sz = snes_saveState(ff4_snes, NULL);
        uint8_t *buf = malloc((size_t)sz);
        if (buf) {
            snes_saveState(ff4_snes, buf);
            FILE *o = fopen(save_path, "wb");
            if (o) { fwrite(buf, (size_t)sz, 1, o); fclose(o);
                     printf("state saved    : %s (%d bytes)\n", save_path, sz); }
            free(buf);
        }
    }

    ff4_shutdown();
    free(rom);
    return stalled ? 3 : 0;   /* scriptable verdict: 0=clean run, 3=stalled/hang */
}
