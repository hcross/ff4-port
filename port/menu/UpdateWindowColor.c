#include "snes/snes.h"

// Entry mode: X 16-bit (xf=0), DB=$FE, DP=0
// Logic: Reads the window color from WRAM $16AA and propagates it to 
// a series of 16-bit registers in the $A000 range.
static void UpdateWindowColor_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // ldx $16aa (X is 16-bit)
    uint16_t color = read16(ram, 0x16AA);
    
    // Sequence of stx $XXXX
    write16(ram, 0xA002, color);
    write16(ram, 0xA00A, color);
    write16(ram, 0xA012, color);
    write16(ram, 0xA01A, color);
    write16(ram, 0xA042, color);
    write16(ram, 0xA04A, color);
    write16(ram, 0xA052, color);
    write16(ram, 0xA05A, color);
    write16(ram, 0xA082, color);
    write16(ram, 0xA08A, color);
    write16(ram, 0xA092, color);
    write16(ram, 0xA09A, color);
    write16(ram, 0xA0C2, color);
    write16(ram, 0xA0CA, color);
    write16(ram, 0xA0D2, color);
    write16(ram, 0xA0DA, color);
}

// PITFALLS: 1 (DB=$FE required for this module)
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x16AA=2
//   output_ram:  0xA002=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFE
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: menu::UpdateWindowColor ($FE:D6)