/* FF4 desktop validation host — headless harness (M0/M1/M2a).
 *
 * See docs/adr/0001-desktop-validation-in-re-loop.md. Runs the live ff4-gnw
 * working tree on desktop through the same ff4_* glue the G&W device uses.
 *
 * Usage:
 *   ff4-desktop-headless <rom.sfc> [flags]
 *     --frames N        run N frames (default 600)
 *     --load  <f.lss>   load a LakeSnes savestate after init (jump to a scene)
 *     --save  <f.lss>   save a savestate after the run (reusable seed)
 *     --out   <f.ppm>   dump the final frame as binary PPM
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "snes/snes.h"   /* Snes, snes_loadState/saveState */

/* ff4-gnw glue (ff4-gnw/main.c) */
extern bool ff4_init(const uint8_t *rom_bytes, int rom_length);
extern void ff4_step(void);
extern void ff4_shutdown(void);
extern void ff4_blit_to_lcd(uint16_t *lcd_fb);
extern Snes *ff4_snes;

/* dispatch counters + runtime toggle (ff4-gnw/dispatch_all.c) */
extern uint32_t ff4_dispatch_hits;
extern uint32_t ff4_dispatch_misses;
extern int ff4_dispatch_enabled;

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
            "usage: %s <rom.sfc> [--frames N] [--load f.lss] [--save f.lss] [--out f.ppm]\n",
            argv[0]);
        return 2;
    }
    const char *rom_path = argv[1];
    int frames = 600;
    const char *load_path = NULL, *save_path = NULL, *out_ppm = NULL;

    for (int i = 2; i < argc; i++) {
        if      (!strcmp(argv[i], "--frames") && i + 1 < argc) frames = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--load")   && i + 1 < argc) load_path = argv[++i];
        else if (!strcmp(argv[i], "--save")   && i + 1 < argc) save_path = argv[++i];
        else if (!strcmp(argv[i], "--out")    && i + 1 < argc) out_ppm = argv[++i];
        else if (!strcmp(argv[i], "--no-dispatch")) ff4_dispatch_enabled = 0;
        else { fprintf(stderr, "error: bad arg '%s'\n", argv[i]); return 2; }
    }

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
    }

    printf("running %d frames...\n", frames);
    for (int i = 0; i < frames; i++) {
        ff4_step();
        if ((i + 1) % 60 == 0)
            printf("  frame %4d | pc=%02X:%04X | hits=%u misses=%u\n",
                   i + 1, ff4_snes->cpu->k, ff4_snes->cpu->pc,
                   ff4_dispatch_hits, ff4_dispatch_misses);
    }

    uint32_t wram_crc = crc32(ff4_snes->ram, sizeof(ff4_snes->ram));
    uint32_t total = ff4_dispatch_hits + ff4_dispatch_misses;
    double hit_rate = total ? (100.0 * ff4_dispatch_hits / total) : 0.0;

    printf("\n=== result ===\n");
    printf("frames run     : %u\n", ff4_snes->frames);
    printf("cpu cycles     : %llu\n", (unsigned long long)ff4_snes->cycles);
    printf("dispatch       : %.1f%% (%u/%u)\n", hit_rate, ff4_dispatch_hits, total);
    printf("WRAM crc32     : %08X\n", wram_crc);

    if (out_ppm && dump_ppm(out_ppm))
        printf("frame dumped   : %s (%dx%d)\n", out_ppm, LCD_W, LCD_H);

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
    return 0;
}
