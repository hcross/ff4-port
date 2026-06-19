/* FF4 desktop — input-path regression probe (F5).
 *
 * Reproduces the SDL host's "input dead in interpreter mode" bug headlessly:
 * boot to the title in native dispatch, snapshot, then inject button A and run
 * a few frames under native vs pure-interpreter from the SAME state, dumping
 * the auto-joypad result and the WRAM joypad mirror so the divergence (if any)
 * is explicit.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "snes/snes.h"

extern bool ff4_init(const uint8_t *rom_bytes, int rom_length);
extern void ff4_step(void);
extern void ff4_set_button(int player, int button, bool pressed);
extern Snes *ff4_snes;
extern int  ff4_dispatch_enabled;
extern int  (*ff4_dispatch_filter)(uint32_t pc);

/* "interpreter except input": keep the reimplemented controller hooks native
 * (they read portAutoRead and write the WRAM mirror — the original asm path is
 * incompatible with the auto-joypad-enabled harness), interpret everything
 * else. 018010 = UpdateCtrlField_ext, 14fd03 = UpdateCtrl_ext (mirror writers),
 * 14fd00 = InitCtrl_ext2. */
static int keep_input_native(uint32_t pc) {
    return pc == 0x018010 || pc == 0x14fd03 || pc == 0x14fd00;
}

static uint8_t *read_file(const char *path, long *out_len) {
    FILE *f = fopen(path, "rb"); if (!f) return NULL;
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    if (n <= 0) { fclose(f); return NULL; }
    uint8_t *b = malloc((size_t)n);
    if (!b || fread(b, 1, (size_t)n, f) != (size_t)n) { free(b); fclose(f); return NULL; }
    fclose(f); *out_len = n; return b;
}

static void dump(const char *tag) {
    Snes *s = ff4_snes;
    uint16_t d = s->cpu->dp;
    printf("  [%-6s] pc=%02X:%04X dp=%04X autoJoyRead=%d portAutoRead[0]=%04X | "
           "ram[0600..0603]=%02X %02X %02X %02X  ram[0000..0003]=%02X %02X %02X %02X\n",
           tag, s->cpu->k, s->cpu->pc, d, s->autoJoyRead, s->portAutoRead[0],
           s->ram[0x0600], s->ram[0x0601], s->ram[0x0602], s->ram[0x0603],
           s->ram[0x0000], s->ram[0x0001], s->ram[0x0002], s->ram[0x0003]);
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s <rom.sfc> [--load f.lss] [--boot N] [--btn N]\n", argv[0]); return 2; }
    const char *rom_path = argv[1], *load_path = NULL;
    int boot = 300, btn = 8;  /* 8 = A (the title polls A at $0602) */
    for (int i = 2; i < argc; i++) {
        if      (!strcmp(argv[i], "--load") && i+1 < argc) load_path = argv[++i];
        else if (!strcmp(argv[i], "--boot") && i+1 < argc) boot = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--btn")  && i+1 < argc) btn  = atoi(argv[++i]);
        else { fprintf(stderr, "bad arg %s\n", argv[i]); return 2; }
    }

    long rl = 0; uint8_t *rom = read_file(rom_path, &rl);
    if (!rom || !ff4_init(rom, (int)rl)) { fprintf(stderr, "init failed\n"); return 1; }
    if (load_path) {
        long n = 0; uint8_t *st = read_file(load_path, &n);
        if (st && snes_loadState(ff4_snes, st, (int)n)) printf("loaded %s\n", load_path);
        free(st);
    }

    printf("booting %d frames (native, no input)...\n", boot);
    ff4_dispatch_enabled = 1;
    for (int i = 0; i < boot; i++) ff4_step();
    dump("boot");

    /* Snapshot the post-boot title state. */
    int sz = snes_saveState(ff4_snes, NULL);
    uint8_t *s0 = malloc((size_t)sz);
    snes_saveState(ff4_snes, s0);

    const int hold = 8;

    /* NATIVE: hold A, run frames. */
    printf("\nNATIVE (dispatch ON), holding btn %d for %d frames:\n", btn, hold);
    ff4_dispatch_enabled = 1;
    ff4_set_button(1, btn, true);
    for (int i = 0; i < hold; i++) { ff4_step(); }
    dump("native");

    /* INTERPRETER: restore, hold A, run frames. */
    printf("\nINTERPRETER (dispatch OFF), holding btn %d for %d frames:\n", btn, hold);
    snes_loadState(ff4_snes, s0, sz);
    ff4_dispatch_enabled = 0;
    ff4_set_button(1, btn, true);
    for (int i = 0; i < hold; i++) { ff4_step(); }
    dump("interp");

    /* PROPOSED FIX: interpret everything EXCEPT the input hooks (kept native
     * via the per-hook filter). dispatch stays enabled; the filter denies all
     * but the input writers. */
    printf("\nINTERP-EXCEPT-INPUT (filter keeps input native), holding btn %d:\n", btn);
    snes_loadState(ff4_snes, s0, sz);
    ff4_dispatch_enabled = 1;
    ff4_dispatch_filter = keep_input_native;
    ff4_set_button(1, btn, true);
    for (int i = 0; i < hold; i++) { ff4_step(); }
    dump("fix");
    ff4_dispatch_filter = 0;

    free(s0); free(rom);
    return 0;
}
