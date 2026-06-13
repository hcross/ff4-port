#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$A5, DP=0
// Logic:
//   Calculates an index by subtracting ram[0x79] from 0x28.
//   Uses this index to look up a Y-offset in WhaleLiftoffYTbl (ROM) 
//   and stores the result in ram[0xB9].
static void _00a51a_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda #$28 / sec / sbc $79
    // Pitfall 7: Result must be truncated to 8-bit to match 65816 A-8 mode
    uint8_t index = (uint8_t)(0x28 - ram[0x79]);

    // lda f:WhaleLiftoffYTbl,x
    // Use the harness-provided rom_read8 for ROM bank access
    uint8_t val = rom_read8(snes, WHALE_LIFTOFF_Y_TBL + index);

    // sta $b9
    ram[0xB9] = val;
}

// PITFALLS: 7 (8-bit arithmetic truncation)
// HELPERS: rom_read8 (harness utility for ROM access)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x79=1
//   output_ram:  0xB9=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xA5
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00a51a ($A5:1A)