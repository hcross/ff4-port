#include "snes/snes.h"

// BtnAction is a data table, not a subroutine. The original asm is:
//   .word $0000,$0080,$8000,$0040
// This C function emulates an indexed read from that table,
// matching the ROM data for parity comparison.
static uint16_t BtnAction_c(Snes *snes, uint8_t index) {
    static const uint16_t table[4] = {0x0000, 0x0080, 0x8000, 0x0040};
    return table[index & 3];
}

// PITFALLS: none (data table, no instructions)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=index (8-bit)
//   inputs_ram:  none
//   output_ram:  none
//   output_reg:  a=value (16-bit)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E (assumed for menu module)
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (data table, not a callable routine; harness must
//   compare ROM bytes or use accessor function)
// REVERSED_FUNCTION: menu::BtnAction ($FE:7E)