/* state_inject — build a .lss savestate from a live-device state capture.
 *
 * Companion to scripts/... none: the capture side is a GDB batch attached to
 * the running G&W (gnwmanager gdbserver; attach halts, `monitor resume`
 * resumes -- no reset, the game keeps its state). The GDB script dumps the
 * big arrays as raw little-endian binaries and EMITS C ASSIGNMENTS for every
 * scalar field that snes_saveState serializes (S->hPos = 123; etc.), which
 * this tool #includes verbatim -- no manifest parsing, no schema drift: the
 * field list lives in one place (the GDB script) and the compiler checks it
 * against the real struct definitions.
 *
 * Usage:
 *   ./ff4-state-inject <rom.sfc> <capture_dir> <out.lss>
 * Build (capture dir provides device_state.inc at compile time):
 *   make ff4-state-inject CAPTURE_DIR=/tmp/ff4cap
 *
 * Known limitation: APU internals (SPC700 regs/RAM, DSP, timers) are left at
 * fresh-init defaults -- only the game-visible mailbox ports are restored.
 * Sound is stubbed everywhere in this port (ExecSound fakes the handshake in
 * WRAM), so game logic is unaffected; do not use such a fixture to validate
 * audio behaviour.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "snes/snes.h"

extern bool ff4_init(const uint8_t *rom_bytes, int rom_length);
extern Snes *ff4_snes;

static uint8_t *read_file(const char *path, long *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *buf = malloc((size_t)n);
    if (!buf || fread(buf, 1, (size_t)n, f) != (size_t)n) { free(buf); fclose(f); return NULL; }
    fclose(f); *out_len = n; return buf;
}

static int slurp_into(const char *dir, const char *name, void *dst, size_t want) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    long n = 0;
    uint8_t *buf = read_file(path, &n);
    if (!buf) { fprintf(stderr, "error: cannot read %s\n", path); return 0; }
    if ((size_t)n != want) {
        fprintf(stderr, "error: %s is %ld bytes, expected %zu\n", path, n, want);
        free(buf); return 0;
    }
    memcpy(dst, buf, want);
    free(buf);
    return 1;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "usage: %s <rom.sfc> <capture_dir> <out.lss>\n", argv[0]);
        return 2;
    }
    const char *rom_path = argv[1], *cap = argv[2], *out_path = argv[3];

    long rom_len = 0;
    uint8_t *rom = read_file(rom_path, &rom_len);
    if (!rom) { fprintf(stderr, "error: cannot read ROM '%s'\n", rom_path); return 1; }
    if (!ff4_init(rom, (int)rom_len)) { fprintf(stderr, "error: ff4_init failed\n"); return 1; }

    Snes  *S  = ff4_snes;
    Cpu   *C  = S->cpu;
    Ppu   *P  = S->ppu;
    Dma   *D  = S->dma;
    Input *I1 = S->input1;
    Input *I2 = S->input2;

    /* Big arrays (raw little-endian device dumps; both sides little-endian). */
    if (!slurp_into(cap, "ram.bin",     S->ram,                0x20000)) return 1;
    if (!slurp_into(cap, "vram.bin",    P->vram,               0x10000)) return 1;
    if (!slurp_into(cap, "cgram.bin",   P->cgram,              0x200))   return 1;
    if (!slurp_into(cap, "oam.bin",     P->oam,                0x200))   return 1;
    if (!slurp_into(cap, "highoam.bin", P->highOam,            0x20))    return 1;
    if (!slurp_into(cap, "objpix.bin",  P->objPixelBuffer,     0x100))   return 1;
    if (!slurp_into(cap, "objprio.bin", P->objPriorityBuffer,  0x100))   return 1;
    if (S->cart->ram != NULL) {
        if (!slurp_into(cap, "sram.bin", S->cart->ram, (size_t)S->cart->ramSize)) return 1;
    }

    /* Scalar fields, emitted by the GDB capture script as C assignments. */
#include "device_state.inc"

    int size = snes_saveState(S, NULL);
    if (size <= 0) { fprintf(stderr, "error: snes_saveState sizing failed\n"); return 1; }
    uint8_t *out = malloc((size_t)size);
    int written = snes_saveState(S, out);
    if (written != size) { fprintf(stderr, "error: save size mismatch (%d vs %d)\n", written, size); return 1; }

    FILE *o = fopen(out_path, "wb");
    if (!o) { fprintf(stderr, "error: cannot write '%s'\n", out_path); return 1; }
    fwrite(out, 1, (size_t)written, o);
    fclose(o);
    printf("wrote %s (%d bytes) | frames=%u pc=%02X:%04X\n",
           out_path, written, S->frames, C->k, C->pc);
    return 0;
}
