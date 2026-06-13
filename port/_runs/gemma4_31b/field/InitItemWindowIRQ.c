#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B0, DP=0
// Purpose: Initialize hardware registers and memory flags for the item window IRQ/NMI sequence.
// Note: This routine uses a mix of symbolic labels (hVTIMEL, etc.) and absolute addresses.
// Based on standard FFIV mappings: 
// hVTIMEL = $008F, hNMITIMEN = $00A1, hBG3HOFS = $00A2, hBG3VOFS = $00A4
static void InitItemWindowIRQ_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // ldx #$008f / stx hVTIMEL
    // Note: X is 16-bit, but target is 8-bit. Only low byte is stored.
    ram[0x008F] = 0x8F;

    // lda #$a1 / sta hNMITIMEN
    ram[0x00A1] = 0xA1;

    // stz hBG3HOFS (twice)
    ram[0x00A2] = 0;
    ram[0x00A2] = 0;

    // lda #$88 / sta hBG3VOFS / stz hBG3VOFS
    // This sequence sets the high byte of a 16-bit value to 0 and low to 0x88
    ram[0x00A4] = 0x88;
    ram[0x00A5] = 0;

    // stz $7f
    ram[0x007F] = 0;

    // ldx $08f6 / inx / stx $08f6
    // X is 16-bit. read16 loads $08f6 and $08f7.
    uint16_t x = read16(ram, 0x08F6);
    x++;
    write16(ram, 0x08F6, x);
}

// PITFALLS: None relevant (straight-line code, no conditional branches).
// HELPERS: read16/write16 — little-endian 16-bit accessors over ram[]
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x08F6=2
//   output_ram:  0x008F=1, 0x00A1=1, 0x00A2=1, 0x00A4=1, 0x00A5=1, 0x007F=1, 0x08F6=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::InitItemWindowIRQ ($B0:AC)