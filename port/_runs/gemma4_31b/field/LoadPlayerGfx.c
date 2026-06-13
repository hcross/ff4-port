#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00, DP=0
// Determines the player's graphic index based on the TopChar properties
// and sets up the transfer parameters for Tfr3bppGfx.
static void LoadPlayerGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if (ram[0xCC] == 0) { // lda $cc / bne @c890
        return;
    }

    ram[0xCC] = 0; // stz $cc
    
    GetTopCharPtr_emu(snes); // jsl GetTopCharPtr (X updated by helper)
    uint16_t x = snes->cpu->x;

    uint8_t gfx_idx;
    // Use (x & 0xFF) for 8-bit indexed addressing if X is 16-bit but offset is 8-bit
    // However, $1003,x is typically an absolute indexed access. 
    // Based on 65816 logic: addr = 0x1003 + x.
    uint8_t prop = ram[0x1003 + x]; // lda $1003,x

    if (prop & 0x20) { // and #$20 / beq @c8a2
        gfx_idx = 0x0F;
    } else if (prop & 0x10) { // and #$10 / beq @c8ae
        gfx_idx = 0x0E;
    } else if (prop & 0x08) { // and #$08 / beq @c8ba
        gfx_idx = 0x10;
    } else {
        uint8_t prop_low = ram[0x1001 + x]; // lda $1001,x
        gfx_idx = prop_low & 0x1F;          // and #$1f
    }

    ram[0x06] = gfx_idx; // sta $06

    // Pitfall 7: Manual 8-bit arithmetic truncation
    uint8_t result = (uint8_t)(gfx_idx << 1);        // asl
    result = (uint8_t)(result + ram[0x06]);          // clc / adc $06
    result = (uint8_t)(result + 0x80);               // clc / adc #$80

    ram[0x4B] = result; // sta $4b
    ram[0x4A] = 0;      // stz $4a

    write16(ram, 0x4C, 0x4000); // ldx #$4000 / stx $4c
    write16(ram, 0x4E, 0x0200); // ldx #$0200 / stx $4e

    // .bankbyte(MapSpriteGfx) resolved to 0x0F per project mapping
    ram[0x49] = 0x0F; 
    
    Tfr3bppGfx_emu(snes); // jsl Tfr3bppGfx
}

// PITFALLS: 7 (8-bit truncation on ASL/ADC chain)
// HELPERS: GetTopCharPtr_emu(snes), Tfr3bppGfx_emu(snes), write16
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x00CC=1, 0x1001=1, 0x1003=1
//   output_ram:  0x004B=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LoadPlayerGfx ($C8:8B)