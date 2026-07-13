/* FF4 desktop miss profiler.
 *
 * Boots the ROM cold (or from a seed) for N frames and records every PC that
 * falls through to the interpreter, sorted by call frequency.  Use this to
 * identify the hot routines to port next.
 *
 * Usage:
 *   ff4-miss-profiler <rom.sfc> [--frames N] [--load f.lss] [--save-seed f.lss]
 *                               [--top K] [--out f.ppm]
 *                               [--walk-square START] [--walk-lr START[:HOLD]]
 *
 * --walk-square mirrors the device's FF4_AUTO_WALK (DPAD square, 30 frames
 * per side, looping). --walk-lr holds left then right for HOLD frames each
 * (default 60), looping -- the free-roam-style workload that exercises the
 * event/NPC interpreter paths the square never touches (2026-07-12).
 *
 * Output: ranked miss list  "count  PC  bank" to stdout.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "snes/snes.h"

extern bool     ff4_init(const uint8_t *rom_bytes, int rom_length);
extern void     ff4_step(void);
extern void     ff4_shutdown(void);
extern void     ff4_blit_to_lcd(uint16_t *lcd_fb);
extern Snes    *ff4_snes;
extern uint32_t ff4_dispatch_hits;
extern uint32_t ff4_dispatch_misses;
extern int      ff4_dispatch_enabled;
extern void   (*ff4_dispatch_miss_trace)(uint32_t pc);
extern void     ff4_set_button(int player, int button, bool pressed);

/* ---- miss PC frequency table -------------------------------------------- */
#define MISS_CAP 4096
typedef struct { uint32_t pc; uint32_t count; } MissEntry;
static MissEntry g_miss[MISS_CAP];
static int       g_miss_n = 0;

static void record_miss(uint32_t pc) {
    for (int i = 0; i < g_miss_n; i++) {
        if (g_miss[i].pc == pc) { g_miss[i].count++; return; }
    }
    if (g_miss_n < MISS_CAP) {
        g_miss[g_miss_n].pc    = pc;
        g_miss[g_miss_n].count = 1;
        g_miss_n++;
    }
}

static int cmp_desc(const void *a, const void *b) {
    return (int)((const MissEntry *)b)->count - (int)((const MissEntry *)a)->count;
}

/* ---- helpers ------------------------------------------------------------ */
static uint8_t *read_file(const char *path, long *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    if (n <= 0) { fclose(f); return NULL; }
    uint8_t *buf = malloc((size_t)n);
    if (!buf || fread(buf, 1, (size_t)n, f) != (size_t)n) { free(buf); fclose(f); return NULL; }
    fclose(f); *out_len = n; return buf;
}

static int dump_ppm(const char *path) {
    static uint16_t fb[320 * 240];
    memset(fb, 0, sizeof(fb));
    ff4_blit_to_lcd(fb);
    FILE *o = fopen(path, "wb"); if (!o) return 0;
    fprintf(o, "P6\n320 240\n255\n");
    for (int i = 0; i < 320 * 240; i++) {
        uint16_t px = fb[i];
        uint8_t r = (uint8_t)(((px >> 11) & 0x1F) << 3);
        uint8_t g = (uint8_t)(((px >>  5) & 0x3F) << 2);
        uint8_t b = (uint8_t)(((px      ) & 0x1F) << 3);
        fwrite((uint8_t[]){r, g, b}, 1, 3, o);
    }
    fclose(o); return 1;
}

/* ---- main --------------------------------------------------------------- */
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,
            "usage: %s <rom.sfc> [--frames N] [--load f.lss] [--save-seed f.lss]"
            " [--top K] [--out f.ppm]\n", argv[0]);
        return 2;
    }
    const char *rom_path  = argv[1];
    int         frames    = 200;
    int         top_k     = 40;
    const char *load_path = NULL, *save_path = NULL, *out_ppm = NULL;
    int         walk_square = 0, walk_lr = 0, walk_lr_hold = 60;

    for (int i = 2; i < argc; i++) {
        if      (!strcmp(argv[i], "--frames")    && i+1 < argc) frames    = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--top")       && i+1 < argc) top_k     = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--load")      && i+1 < argc) load_path = argv[++i];
        else if (!strcmp(argv[i], "--save-seed") && i+1 < argc) save_path = argv[++i];
        else if (!strcmp(argv[i], "--out")       && i+1 < argc) out_ppm   = argv[++i];
        else if (!strcmp(argv[i], "--walk-square") && i+1 < argc) walk_square = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--walk-lr")   && i+1 < argc) {
            char *spec = argv[++i];
            char *colon = strchr(spec, ':');
            if (colon) { *colon = '\0'; walk_lr_hold = atoi(colon + 1); }
            walk_lr = atoi(spec);
            if (walk_lr_hold < 1) walk_lr_hold = 60;
        }
        else { fprintf(stderr, "error: bad arg '%s'\n", argv[i]); return 2; }
    }

    long rom_len = 0;
    uint8_t *rom = read_file(rom_path, &rom_len);
    if (!rom) { fprintf(stderr, "error: cannot read ROM '%s'\n", rom_path); return 1; }

    if (!ff4_init(rom, (int)rom_len)) {
        fprintf(stderr, "error: ff4_init failed\n"); free(rom); return 1;
    }

    if (load_path) {
        long st_len = 0;
        uint8_t *st = read_file(load_path, &st_len);
        if (!st) { fprintf(stderr, "error: cannot read state '%s'\n", load_path); free(rom); return 1; }
        if (!snes_loadState(ff4_snes, st, (int)st_len)) {
            fprintf(stderr, "error: snes_loadState rejected '%s'\n", load_path); free(st); free(rom); return 1;
        }
        free(st);
        printf("loaded seed: %s  pc=%02X:%04X\n", load_path,
               ff4_snes->cpu->k, ff4_snes->cpu->pc);
    }

    ff4_dispatch_miss_trace = record_miss;

    printf("profiling %d frames (dispatch ON)...\n", frames);
    for (int i = 0; i < frames; i++) {
        if (walk_square && i + 1 >= walk_square) {
            static const int btn4[4] = { 4 /*up*/, 7 /*right*/, 5 /*down*/, 6 /*left*/ };
            const int t = (i + 1 - walk_square) % 120;
            const int phase = t / 30;
            if (t % 30 == 0) {
                ff4_set_button(1, btn4[(phase + 3) & 3], false);
                ff4_set_button(1, btn4[phase], true);
            }
        }
        if (walk_lr && i + 1 >= walk_lr) {
            const int t = (i + 1 - walk_lr) % (2 * walk_lr_hold);
            if (t == 0)            { ff4_set_button(1, 7, false); ff4_set_button(1, 6, true); }  /* left  */
            else if (t == walk_lr_hold) { ff4_set_button(1, 6, false); ff4_set_button(1, 7, true); }  /* right */
        }
        ff4_step();
        if ((i + 1) % 60 == 0)
            printf("  frame %4d | pc=%02X:%04X | hits=%u misses=%u unique_miss_pcs=%d\n",
                   i+1, ff4_snes->cpu->k, ff4_snes->cpu->pc,
                   ff4_dispatch_hits, ff4_dispatch_misses, g_miss_n);
    }

    if (out_ppm && dump_ppm(out_ppm))
        printf("frame dumped: %s\n", out_ppm);

    if (save_path) {
        int sz = snes_saveState(ff4_snes, NULL);
        uint8_t *buf = malloc((size_t)sz);
        if (buf) {
            snes_saveState(ff4_snes, buf);
            FILE *o = fopen(save_path, "wb");
            if (o) { fwrite(buf, 1, (size_t)sz, o); fclose(o);
                     printf("seed saved: %s (%d bytes)\n", save_path, sz); }
            free(buf);
        }
    }

    qsort(g_miss, g_miss_n, sizeof(MissEntry), cmp_desc);

    uint32_t total = ff4_dispatch_hits + ff4_dispatch_misses;
    printf("\n=== miss profile — top %d / %d unique PCs (%u total misses, %.1f%% miss rate) ===\n",
           top_k < g_miss_n ? top_k : g_miss_n, g_miss_n,
           ff4_dispatch_misses,
           total ? 100.0 * ff4_dispatch_misses / total : 0.0);
    printf("  %-8s  %-10s  %s\n", "count", "pc", "bank");
    printf("  %-8s  %-10s  %s\n", "--------", "----------", "----");
    int show = top_k < g_miss_n ? top_k : g_miss_n;
    for (int i = 0; i < show; i++) {
        printf("  %-8u  %06X      $%02X\n",
               g_miss[i].count, g_miss[i].pc, (g_miss[i].pc >> 16) & 0xff);
    }

    ff4_shutdown();
    free(rom);
    return 0;
}
