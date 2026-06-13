#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x0, DP=0
// Logic:
//   This routine initializes several system/DMA parameters in WRAM
//   (likely for a DMA transfer to restore dialog graphics).
//   It sets a start address (0x2000), a destination (0xe600), 
//   the source bank (0x7E), and a length (0x1000).
static void RestoreDlgGfx_ext_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda #$00 / pha / plb
    // This sequence clears the B register (the high byte of A in 8-bit mode)
    // by pushing 0 to the stack and pulling it into B.
    snes->cpu->a = (snes->cpu->a & 0x00FF); 

    // ldx #$2000 / stx $011d
    write16(ram, 0x011D, 0x2000);

    // ldx #$e600 / stx $011f
    write16(ram, 0x011F, 0xE600);

    // lda #$7e / sta $0121
    ram[0x0121] = 0x7E;

    // ldx #$1000 / stx $0122
    write16(ram, 0x0122, 0x1000);
}

// PITFALLS: 9 (pha/plb explicitly clears the B register, ensuring 
// 16-bit writes via X are zero-extended from the provided constants)
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x011D=2, 0x011F=2, 0x0121=1, 0x0122=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: menu::RestoreDlgGfx_ext ($FF:D6)