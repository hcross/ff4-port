#include "snes/snes.h"

// LoadTheEndGfx: Loads "The End" graphics into VRAM for end credits.
// Entry mode: A 16-bit (mf=0), X 16-bit (xf=0), DB=any, DP=0
// Entry condition: routine checks $64 == 2 to proceed
static void LoadTheEndGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    // cmp #2 / bne @d763
    if (ram[0x64] != 2) return;

    // ldx #$001b / stx hBG1SC
    write16(ram, 0x2108, 0x001B);

    // phb / clr_a / pha / plb
    uint8_t old_db = snes->cpu->db;
    snes->cpu->db = 0x00;  // clr_a = tdc; pha/plb sets DB to 0

    // ldx #$3000 / stx hVMADDL
    write16(ram, 0x2116, 0x3000);

    // clr_ax (tdc / tax)
    uint16_t a = 0;
    uint16_t x = 0;

    // Loop from x=0 to x=0x320 (0x320 iterations)
    // Each iteration reads one byte from f:TheEndGfx,x,
    // writes low nybble to VRAM, then high nybble >> 4 to VRAM
    for (x = 0; x < 0x0320; x++) {
        // lda f:TheEndGfx,x (data bank is 0 now)
        uint8_t val = ram[0x3000 + x];  // f:TheEndGfx is at $00:3000

        // and #$0f / sta hVMDATAH
        ram[0x2119] = val & 0x0F;

        // pla / and #$f0 / lsr4 / sta hVMDATAH
        ram[0x2119] = (val >> 4) & 0x0F;
    }

    // plb
    snes->cpu->db = old_db;
}

// PITFALLS: 1 (DB register manipulated), 6 (A is 16-bit on entry),
//           8 (X is 16-bit due to longi in caller), 9 (clr_ax is tdc/tax)
// HELPERS: none
// CONTRACT:
//   inputs_ram: 0x64=1
//   output_ram: none
//   entry_mode: mf=false, xf=false, dp=0x0, db=any
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::LoadTheEndGfx ($D7:30)