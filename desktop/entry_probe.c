/* FF4 desktop entry-state probe.
 *
 * Runs a seed for N frames with dispatch enabled and records, for every
 * dispatched hit, the CPU entry state the native body actually receives:
 * D (direct page), DB, and the m/x widths. One distinct (pc, D, DB, m, x)
 * tuple per output line, with its hit count.
 *
 * Why: a routine's CONTRACT declares dp=/db=/mf=/xf= and its spike replays
 * exactly that state -- a green spike under a WRONG declared D compares C
 * and asm in a world the game never enters (AGENTS.md golden rule 1,
 * ff4-gnw/CONVENTIONS.md). This measures the real entry state of every
 * dispatched routine in one run instead of one printf trace per routine;
 * registry/check_entry_state.py cross-checks the result against the
 * CONTRACTs.
 *
 * Usage:
 *   ff4-entry-probe <rom.sfc> --load f.lss [--frames N] [--walk-square START]
 * --walk-square mirrors harness_headless.c (DPAD square, 30 frames a side):
 * field movement routines only run while the player walks.
 * Output (stdout): "ENTRY <pc6> dp=<hex4> db=<hex2> m=<0|1> x=<0|1> hits=<n>"
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "snes/snes.h"
#include "snes/cpu.h"

extern bool     ff4_init(const uint8_t *rom_bytes, int rom_length);
extern void     ff4_step(void);
extern Snes    *ff4_snes;
extern void   (*ff4_dispatch_trace)(uint32_t pc);
extern void     ff4_set_button(int player, int button, bool pressed);

#define CAP 4096
typedef struct { uint32_t pc; uint16_t dp; uint8_t db, m, x; uint32_t hits; } Entry;
static Entry g_e[CAP];
static int   g_n;

static void trace_cb(uint32_t pc) {
    const Cpu *c = ff4_snes->cpu;
    uint8_t m = c->mf ? 1 : 0, x = c->xf ? 1 : 0;
    for (int i = 0; i < g_n; i++) {
        Entry *e = &g_e[i];
        if (e->pc == pc && e->dp == c->dp && e->db == c->db && e->m == m && e->x == x) {
            e->hits++;
            return;
        }
    }
    if (g_n < CAP) g_e[g_n++] = (Entry){ pc, c->dp, c->db, m, x, 1 };
}

static uint8_t *read_file(const char *path, long *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    if (n <= 0) { fclose(f); return NULL; }
    uint8_t *buf = malloc((size_t)n);
    if (!buf || fread(buf, 1, (size_t)n, f) != (size_t)n) { free(buf); fclose(f); return NULL; }
    fclose(f); *out_len = n; return buf;
}

static int cmp_pc(const void *a, const void *b) {
    const Entry *x = a, *y = b;
    if (x->pc != y->pc) return x->pc < y->pc ? -1 : 1;
    return (int)y->hits - (int)x->hits;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <rom.sfc> --load f.lss [--frames N] [--walk-square START]\n", argv[0]);
        return 2;
    }
    const char *load_path = NULL;
    int frames = 600, walk_start = 0;
    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "--load") && i + 1 < argc) load_path = argv[++i];
        else if (!strcmp(argv[i], "--frames") && i + 1 < argc) frames = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--walk-square") && i + 1 < argc) walk_start = atoi(argv[++i]);
        else { fprintf(stderr, "error: bad arg '%s'\n", argv[i]); return 2; }
    }
    long rom_len = 0;
    uint8_t *rom = read_file(argv[1], &rom_len);
    if (!rom) { fprintf(stderr, "error: cannot read ROM\n"); return 1; }
    if (!ff4_init(rom, (int)rom_len)) { fprintf(stderr, "error: ff4_init failed\n"); return 1; }
    if (load_path) {
        long st_len = 0;
        uint8_t *st = read_file(load_path, &st_len);
        if (!st || !snes_loadState(ff4_snes, st, (int)st_len)) {
            fprintf(stderr, "error: cannot load state '%s'\n", load_path);
            return 1;
        }
        free(st);
    }
    ff4_dispatch_trace = trace_cb;
    static const int walk_btn[4] = { 4 /*up*/, 7 /*right*/, 5 /*down*/, 6 /*left*/ };
    for (int f = 0; f < frames; f++) {
        if (walk_start && f + 1 >= walk_start) {
            const int t = (f + 1 - walk_start) % 120, phase = t / 30;
            if (t % 30 == 0) {
                ff4_set_button(1, walk_btn[(phase + 3) & 3], false);
                ff4_set_button(1, walk_btn[phase], true);
            }
        }
        ff4_step();
    }
    ff4_dispatch_trace = NULL;

    qsort(g_e, (size_t)g_n, sizeof(Entry), cmp_pc);
    for (int i = 0; i < g_n; i++)
        printf("ENTRY %06X dp=%04X db=%02X m=%u x=%u hits=%u\n",
               g_e[i].pc, g_e[i].dp, g_e[i].db, g_e[i].m, g_e[i].x, g_e[i].hits);
    free(rom);
    return 0;
}
