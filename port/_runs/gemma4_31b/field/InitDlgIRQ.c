#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B6, DP=0
// Purpose: Initializes timing and offsets for the dialog IRQ/NMI handler.
// This routine interacts with both WRAM and hardware-mapped registers.
//
// Note: hVTIMEL, hNMITIMEN, hBG3HOFS, hBG3VOFS are mapped to 
// WRAM addresses in the $B6 bank (or mapped via DP=0 in this specific module).
// Looking at the asm, $08f6 is a WRAM variable.
static void InitDlgIRQ_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // ldx #$0013 / stx hVTIMEL
    // Assuming hVTIMEL is mapped to WRAM offset 0x0012 (based on typical mapping)
    // however, since we are using absolute/DP addressing in the asm:
    write16(ram, 0x0012, 0x0013); 

    // lda #$a1 / sta hNMITIMEN
    ram[0x0013] = 0xA1;

    // stz hBG3HOFS (twice)
    // Since it's STZ (16-bit zero), it clears two bytes.
    ram[0x0014] = 0;
    ram[0x0015] = 0;

    // lda #$03 / sta hBG3VOFS / stz hBG3VOFS
    // The ASM sequence:
    // lda #$03
    // sta hBG3VOFS   -> ram[0x0016] = 3
    // stz hBG3VOFS   -> ram[0x0016] = 0, ram[0x0017] = 0
    // Effectively, the STA is immediately overwritten by the STZ.
    ram[0x0016] = 0;
    ram[0x0017] = 0;

    // stz $7f
    ram[0x7F] = 0;
    ram[0x80] = 0;

    // ldx $08f6 / inx / stx $08f6
    uint16_t val = read16(ram, 0x08F6);
    val++;
    write16(ram, 0x08F6, val);
}

// PITFALLS: 6 (X is 16-bit, so read16/write16 used for $08F6 and hVTIMEL),
// 1 (DB is $B6, though these specific offsets are often treated as 0-page/WRAM)
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x08F6=2
//   output_ram:  0x08F6=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB6
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::InitDlgIRQ ($B6:F1)