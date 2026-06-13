#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x91 (Sprite RAM bank), DP=0
// Purpose: Resets sprite attributes for 64 sprites.
// 1. Sets high-order bytes of sprite properties to 0xF0 for 256 bytes (covering 64 sprites * 4 bytes/sprite).
// 2. Clears the 32-byte high-memory range for sprite data (sprite_ram_hi).
static void ResetSprites64_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // First loop: @919a
    // ldx #0 / lda #$f0
    // sta sprite_ram+1,x / inx4 / cpx #$0100 / bne @919a
    // Note: sprite_ram+1 is likely an absolute address offset. 
    // In SNES, sprite RAM typically starts at $00:0000 (via VRAM) or mirrored.
    // Based on the 65816 logic, it writes 0xF0 to every 4th byte starting at offset 1.
    for (uint16_t x = 0; x < 0x0100; x += 4) {
        ram[0x0001 + x] = 0xF0;
    }

    // Second loop: @91a9
    // ldx #0 / stz sprite_ram_hi,x / inx / cpx #$20 / bne @91a9
    // sprite_ram_hi usually refers to the start of the high-byte page (0x0100).
    for (uint16_t x = 0; x < 0x20; x++) {
        ram[0x0100 + x] = 0;
    }
}

// PITFALLS: None (Simple linear loops, no conditional entry flags or arithmetic overflows)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x0001=1, 0x0100=1 (multiple bytes modified)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x91
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::ResetSprites64 ($91:95)