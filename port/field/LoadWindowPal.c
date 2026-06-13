#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 8-bit (xf=1), DB=0x00, DP=0x00
// Logic: 
//   1. Copies 32 bytes from WindowPal (ROM/Data) to WRAM $0CDB.
//   2. Reads a 16-bit value from WRAM $16AA and writes it to WRAM $0CDD.
static void LoadWindowPal_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // @c226: ldx #0
    // @c229: loop to copy palette
    for (uint8_t x = 0; x < 0x20; x++) {
        // WindowPal is typically a ROM label. In the parity harness, 
        // this is accessed via a pointer to the ROM data.
        // The asm `lda f:WindowPal,x` uses the file/absolute address.
        ram[0x0CDB + x] = snes->rom[WINDOW_PAL_OFFSET + x];
    }

    // ldx $16aa / stx $0cdd
    uint16_t val = read16(ram, 0x16AA);
    write16(ram, 0x0CDD, val);
}

// PITFALLS: None. Direct memory copies and 16-bit register transfer.
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x16AA=2, 0xWINDOW_PAL_OFFSET=32
//   output_ram: 0x0CDB=32, 0x0CDD=2
//   entry_mode:  mf=true, xf=true, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LoadWindowPal ($C2:26)