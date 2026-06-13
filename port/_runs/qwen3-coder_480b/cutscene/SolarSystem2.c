#include "snes/snes.h"

static void SolarSystem2_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    // Entry mode: A 8-bit (shorta), X/Y 16-bit (longi)
    write16(ram, 0xA1, 0x0008);
    write16(ram, 0xA3, 0xFFF2);
    ram[0x64] = 1;      // cutscene id
    ram[0x6A] = 0x80;   // cutscene duration low byte
    ram[0x6B] = 0x0A;   // cutscene duration high byte
    // bra _d66b (unconditional jump to next part of cutscene logic)
    _d66b_emu(snes);
}

// PITFALLS: 1 (DB must be set correctly for absolute stores),
//           8 (mode A/X must match caller expectation)
// HELPERS: _d66b_emu(snes) — delegates _d66b @ $D6:6B
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::SolarSystem2 ($D6:2B)