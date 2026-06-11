// FF4 parity harness — Phase 2 scaffold.
// Headless: loads the rebuilt ROM, runs N frames through LakeSnes, dumps a
// CRC32 signature of the WRAM. Serves as the foundation for the next stages
// (double-instance comparator, frame-by-frame diff against the future native
// C reimplementation).
//
// Usage: ff4-parity <rom.sfc> [frames]
//
// The per-frame WRAM signature is the canary: any behavioural divergence
// between the vanilla ROM and a ROM patched with native C code will show up
// as CRC drift.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "snes.h"

static uint32_t crc32_buf(const uint8_t *data, size_t len) {
  static uint32_t table[256];
  static bool init = false;
  if (!init) {
    for (uint32_t i = 0; i < 256; i++) {
      uint32_t c = i;
      for (int j = 0; j < 8; j++)
        c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
      table[i] = c;
    }
    init = true;
  }
  uint32_t c = 0xFFFFFFFFu;
  for (size_t i = 0; i < len; i++)
    c = table[(c ^ data[i]) & 0xFF] ^ (c >> 8);
  return c ^ 0xFFFFFFFFu;
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

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <rom.sfc> [frames]\n", argv[0]);
    return 1;
  }
  int frames = (argc >= 3) ? atoi(argv[2]) : 600; // 10s @ 60fps
  size_t rom_len = 0;
  uint8_t *rom = read_file(argv[1], &rom_len);
  if (!rom) return 2;

  Snes *snes = snes_init();
  if (!snes_loadRom(snes, rom, rom_len)) {
    fprintf(stderr, "snes_loadRom failed\n");
    free(rom);
    return 3;
  }
  snes_reset(snes, true);

  fprintf(stderr, "parity scaffold: %d frames, WRAM CRC32 sampled every 60f\n", frames);
  for (int f = 0; f < frames; f++) {
    snes_runFrame(snes);
    if ((f % 60) == 0 || f == frames - 1) {
      uint32_t c = crc32_buf(snes->ram, sizeof(snes->ram));
      printf("frame=%5d  wram_crc32=%08X\n", f, c);
    }
  }
  free(rom);
  return 0;
}
