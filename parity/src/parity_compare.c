// FF4 parity harness — double-instance comparator.
// Loads two ROMs (vanilla "or" vs test), runs them in lock-step through
// LakeSnes, and `memcmp`s WRAM(128KB) + SRAM(dyn) + VRAM(64KB) + OAM(512B)
// + CGRAM(512B) frame by frame.
//
// Architecture reference: external/zelda3/zelda_cpu_infra.c VerifySnapshotsEq.
// No whitelist of excluded bytes here — this tool is exactly what lets us
// BUILD one by observing real divergences.
//
// Usage:
//   ff4-parity-compare <rom_or.sfc> <rom_test.sfc> [frames] [--patch OFFSET:HEX]
//
// Flags:
//   --patch OFF:HH  Before loading rom_test, overwrite the byte at OFF
//                   with HH (useful for testing the harness's sensitivity).
//
// Exit: 0 if zero divergence over the run, 1 otherwise.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "snes.h"
#include "ppu.h"
#include "cart.h"

#define MAX_DIVERGENCES_PER_REGION 8
#define MAX_DIVERGENCES_TOTAL      256

typedef struct {
  const char *name;
  const void *a;
  const void *b;
  size_t size;
  size_t elem_size;       // 1 or 2 (byte vs word)
} Region;

static int g_total_divergences = 0;

static int compare_region(int frame, const Region *r) {
  if (memcmp(r->a, r->b, r->size) == 0) return 0;
  int local = 0;
  if (r->elem_size == 1) {
    const uint8_t *a = r->a, *b = r->b;
    for (size_t i = 0; i < r->size && local < MAX_DIVERGENCES_PER_REGION; i++) {
      if (a[i] != b[i]) {
        if (g_total_divergences++ < MAX_DIVERGENCES_TOTAL)
          fprintf(stderr, "  [%s] @0x%06zX: or=%02X test=%02X\n",
                  r->name, i, a[i], b[i]);
        local++;
      }
    }
  } else {
    const uint16_t *a = r->a, *b = r->b;
    size_t n = r->size / 2;
    for (size_t i = 0; i < n && local < MAX_DIVERGENCES_PER_REGION; i++) {
      if (a[i] != b[i]) {
        if (g_total_divergences++ < MAX_DIVERGENCES_TOTAL)
          fprintf(stderr, "  [%s] @0x%06zX: or=%04X test=%04X\n",
                  r->name, i, a[i], b[i]);
        local++;
      }
    }
  }
  fprintf(stderr, "  [%s] frame=%d: %d divergences shown (region scan)\n",
          r->name, frame, local);
  return local;
}

static uint8_t *read_file(const char *path, size_t *out_len) {
  FILE *f = fopen(path, "rb");
  if (!f) { perror(path); return NULL; }
  fseek(f, 0, SEEK_END);
  long sz = ftell(f);
  fseek(f, 0, SEEK_SET);
  uint8_t *buf = malloc(sz);
  if (!buf) { fclose(f); return NULL; }
  if (fread(buf, 1, sz, f) != (size_t)sz) { free(buf); fclose(f); return NULL; }
  fclose(f);
  *out_len = sz;
  return buf;
}

static void usage(const char *argv0) {
  fprintf(stderr,
    "usage: %s <rom_or.sfc> <rom_test.sfc> [frames] [--patch OFF:HH]\n"
    "  OFF in hex (with or without 0x), HH in hex byte\n"
    "  Default frames = 600 (10s @ 60fps)\n", argv0);
}

int main(int argc, char **argv) {
  if (argc < 3) { usage(argv[0]); return 1; }

  const char *path_or = argv[1];
  const char *path_test = argv[2];
  int frames = 600;
  long patch_off = -1;
  uint8_t patch_val = 0;

  for (int i = 3; i < argc; i++) {
    if (strcmp(argv[i], "--patch") == 0 && i + 1 < argc) {
      char *colon = strchr(argv[++i], ':');
      if (!colon) { usage(argv[0]); return 1; }
      *colon = 0;
      patch_off = strtol(argv[i], NULL, 16);
      patch_val = (uint8_t)strtol(colon + 1, NULL, 16);
    } else {
      frames = atoi(argv[i]);
    }
  }

  size_t len_or = 0, len_test = 0;
  uint8_t *rom_or = read_file(path_or, &len_or);
  uint8_t *rom_test = read_file(path_test, &len_test);
  if (!rom_or || !rom_test) return 2;

  if (patch_off >= 0) {
    if ((size_t)patch_off >= len_test) {
      fprintf(stderr, "patch offset 0x%lX out of ROM bounds (size 0x%zX)\n",
              patch_off, len_test);
      return 1;
    }
    uint8_t before = rom_test[patch_off];
    rom_test[patch_off] = patch_val;
    fprintf(stderr, "patch: rom_test[0x%lX] %02X -> %02X\n",
            patch_off, before, patch_val);
  }

  Snes *snes_or = snes_init();
  Snes *snes_test = snes_init();
  if (!snes_loadRom(snes_or, rom_or, len_or) ||
      !snes_loadRom(snes_test, rom_test, len_test)) {
    fprintf(stderr, "snes_loadRom failed\n");
    return 3;
  }
  snes_reset(snes_or, true);
  snes_reset(snes_test, true);

  // Sanity: both carts must share the same SRAM size.
  if (snes_or->cart->ramSize != snes_test->cart->ramSize) {
    fprintf(stderr, "WARN: SRAM size mismatch or=%u test=%u -- skipping SRAM compare\n",
            snes_or->cart->ramSize, snes_test->cart->ramSize);
  }
  uint32_t sram_size = snes_or->cart->ramSize;

  fprintf(stderr, "comparator: %d frames, regions = WRAM(128KB) SRAM(%uB) VRAM(64KB) OAM(512B) CGRAM(512B)\n",
          frames, sram_size);

  int frames_with_div = 0;
  for (int f = 0; f < frames; f++) {
    snes_runFrame(snes_or);
    snes_runFrame(snes_test);

    Region regions[] = {
      { "WRAM",  snes_or->ram,        snes_test->ram,        sizeof(snes_or->ram), 1 },
      { "SRAM",  snes_or->cart->ram,  snes_test->cart->ram,  sram_size,            1 },
      { "VRAM",  snes_or->ppu->vram,  snes_test->ppu->vram,  sizeof(snes_or->ppu->vram), 2 },
      { "OAM",   snes_or->ppu->oam,   snes_test->ppu->oam,   sizeof(snes_or->ppu->oam), 2 },
      { "CGRAM", snes_or->ppu->cgram, snes_test->ppu->cgram, sizeof(snes_or->ppu->cgram), 2 },
    };

    int div_this_frame = 0;
    for (size_t i = 0; i < sizeof(regions)/sizeof(regions[0]); i++) {
      if (regions[i].size == 0) continue;
      div_this_frame += compare_region(f, &regions[i]);
    }
    if (div_this_frame > 0) {
      frames_with_div++;
      fprintf(stderr, "frame=%5d: %d divergences\n", f, div_this_frame);
    }
    if (g_total_divergences >= MAX_DIVERGENCES_TOTAL) {
      fprintf(stderr, "STOP: total divergence log cap reached (%d), stopping diagnostic\n",
              MAX_DIVERGENCES_TOTAL);
      break;
    }
  }

  free(rom_or); free(rom_test);

  fprintf(stderr, "\n=== summary ===\n");
  fprintf(stderr, "frames examined  : %d\n", frames);
  fprintf(stderr, "frames divergent : %d\n", frames_with_div);
  fprintf(stderr, "total divergences logged : %d (cap %d)\n",
          g_total_divergences, MAX_DIVERGENCES_TOTAL);
  return frames_with_div == 0 ? 0 : 1;
}
