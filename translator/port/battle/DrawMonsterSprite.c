#include "snes/snes.h"
#include "snes/cart.h"

/* Standalone spike harness extraction of DrawMonsterSprite_c (= asm
 * UpdateCharPalette @ $02:DA73), bundled in ff4-gnw/battle/btlgfx_monsters.c.
 * The body is copied verbatim; inject_cycles is stubbed no-op because the
 * region-WRAM spike compares post-state, not cycle timing (a plain ROM→WRAM
 * copy fires no WRAM-writing HDMA/NMI, so cycle count is irrelevant to state). */

#define LOROM(bank, addr) (((uint32_t)(bank) << 15) | ((addr) & 0x7FFF))
static inline void inject_cycles(Snes *snes, int n) { (void)snes; (void)n; }

void DrawMonsterSprite_c(Snes *snes) {
    uint8_t mon = snes->ram[0x47];
    uint8_t tile_base = snes->ram[0xF0A3 + mon];  /* LDA $F0A3,X where X=mon */

    /* Y = (mon + 9) * 32 — OAM slot offset */
    uint16_t y_oam = (uint16_t)((mon + 9) << 5);
    /* X = tile_base * 32 — ROM data offset within $1C:FD00 block */
    uint16_t x_rom = (uint16_t)((uint16_t)tile_base << 5);

    if (!snes->ram[0xF0AD] && !snes->ram[0xF283]) {
        /* Copy 32 bytes from ROM $1C:FD00+x_rom → WRAM $ED50+y_oam */
        uint32_t rom_base = LOROM(0x1C, 0xFD00) + x_rom;
        uint16_t wram_base = 0xED50 + y_oam;
        for (int i = 0; i < 0x20; i++) {
            snes->ram[wram_base + i] = snes->cart->rom[rom_base + i];
        }
    }

    /* Optional: write palette/priority bytes into OAM high table */
    if (snes->ram[0xD7]) {
        if (snes->ram[0x47] == snes->ram[0x1822]) {
            if (snes->ram[0x1813] & 0x04) {
                snes->ram[0xED52 + y_oam] = 0xEF;
                snes->ram[0xED53 + y_oam] = 0x3D;
            } else {
                snes->ram[0xED52 + y_oam] = 0x00;
                snes->ram[0xED53 + y_oam] = 0x00;
            }
        }
    }

    inject_cycles(snes, 0);  /* timing irrelevant for the region spike */
}

// SPIKE_COMPARE: region
// SPIKE_MASK: 0x1C-0x1D
// CONTRACT:
//   inputs_ram:  0x47=1, 0xF0A3=1, 0xF0AD=1, 0xF283=1, 0xD7=1, 0x1822=1, 0x1813=1
//   output_ram:  0xED50=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: btlgfx::UpdateCharPalette ($02:DA73)
