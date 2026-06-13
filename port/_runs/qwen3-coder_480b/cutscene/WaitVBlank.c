#include "snes/snes.h"

// Busy-wait until VBlank. Uses $41 as a flag byte (set to 1, then
// waits until it's cleared by an interrupt). Also calls _13df4d.
static void WaitVBlank_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    // phx (not used, but preserved)
    ram[0x41]++;              // inc $41
    while (ram[0x41] != 0) {} // lda $41 / bne @df44
    _13df4d_emu(snes);        // jsr _13df4d
    // plx / rts (return)
}

// PITFALLS: 1 (DB=$7E required if _13df4d accesses RAM),
//           4 (stack address depends on E flag — but no access here)
// HELPERS: _13df4d_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x41=1
//   output_ram:  0x41=1     // assumed post-condition: cleared by NMI
//   entry_mode:  mf=true, xf=true, dp=0x0, db=0x7E
//   entry_flags: none
// REVERSED_FUNCTION: cutscene::WaitVBlank ($DF:41)