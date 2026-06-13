#include "snes/snes.h"

// Purpose: Sets up DMA parameters to invert palette colors by transferring 
// a block of memory ($BDB0) to the palette RAM area via the DMA engine.
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$EE (Field/System), DP=0
static void TfrInvertPal_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Initialize DMA control and flags
    ram[0x420B] = 0;    // stz $420b
    ram[0x2121] = 0;    // stz $2121

    // DMA Source Address: $BDB0
    ram[0x4300] = 0x02; // sta $4300 (Address low byte)
    ram[0x4301] = 0x22; // sta $4301 (Address mid byte)
    // Note: $4302 is written via X (16-bit)
    write16(ram, 0x4302, 0xBDB); // ldx #$0bdb / stx $4302 (Address high/offset)

    // DMA Destination Address: $0100 (CGRAM/Palette)
    ram[0x4304] = 0x00; // sta $4304
    write16(ram, 0x4305, 0x0100); // ldx #$0100 / stx $4305

    // Trigger DMA transfer
    ram[0x420B] = 0x01; // sta $420b
}

// PITFALLS: 1 (DB=$EE required for these memory accesses)
// HELPERS: read16/write16 — little-endian 16-bit accessors over ram[]
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x420B=1, 0x2121=1, 0x4300=1, 0x4301=1, 0x4302=2, 0x4304=1, 0x4305=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xEE
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::TfrInvertPal ($EE:35)