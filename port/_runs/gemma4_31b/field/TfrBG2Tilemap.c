#include "snes/snes.h"

// This routine configures and triggers two DMA transfers to update the BG2 tilemap.
// It sets the source address (from WRAM), target address, and size before calling ExecDMA.
static void TfrBG2Tilemap_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // First DMA Transfer
    uint16_t src1 = read16(ram, 0x99);         // ldx $99
    write16(ram, 0x2116, src1);               // stx $2116
    ram[0x420B] = 0;                           // stz $420b
    write16(ram, 0x4302, 0x0ADB);              // ldx #$0adb / stx $4302
    write16(ram, 0x4305, 0x0040);              // ldx #$0040 / stx $4305
    ExecDMA_emu(snes);                        // jsr ExecDMA

    // Second DMA Transfer
    uint16_t src2 = read16(ram, 0x9D);         // ldx $9d
    write16(ram, 0x2116, src2);               // stx $2116
    ram[0x420B] = 0;                           // stz $420b
    write16(ram, 0x4302, 0x0B1B);              // ldx #$0b1b / stx $4302
    write16(ram, 0x4305, 0x0040);              // ldx #$0040 / stx $4305
    ExecDMA_emu(snes);                        // jsr ExecDMA
}

// PITFALLS: 1 (DB=$7E assumed for WRAM sources $99 and $9D).
// HELPERS: ExecDMA_emu(snes) — delegates ExecDMA @ $8B36
//
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x99=2, 0x9D=2
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::TfrBG2Tilemap ($FB:93)