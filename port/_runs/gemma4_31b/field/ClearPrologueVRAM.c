#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$86, DP=0
// Sets bounds in DP $45 and $47 to clear specific VRAM regions via ClearVRAM.
static void ClearPrologueVRAM_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // sta $76
    ram[0x76] = (uint8_t)snes->cpu->a;

    // ldx #$1800 / stx $47
    write16(ram, 0x47, 0x1800);
    // ldx #$1000 / stx $45
    write16(ram, 0x45, 0x1000);
    
    ClearVRAM_emu(snes); // jsl ClearVRAM

    // ldx #$3000 / stx $47
    write16(ram, 0x47, 0x3000);
    
    ClearVRAM_emu(snes); // jsl ClearVRAM
}

// PITFALLS: 1 (DB=$86 used for DP access)
// HELPERS: ClearVRAM_emu(snes) — delegates ClearVRAM @ $83:E1
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x76=1, 0x45=2, 0x47=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x86
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::ClearPrologueVRAM ($86:A8)