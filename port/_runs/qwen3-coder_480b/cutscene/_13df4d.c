#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: A = ram[$64] (caller must set Z/N to reflect this — see pitfall 2)
// Logic:
//   if (ram[$64] == 2) {
//     call _13ec81
//   }
//   return
static void _13df4d_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = ram[0x64];         // lda $64
    if (a == 2) {                  // cmp #2 / bne @df56
        _13ec81_emu(snes);         // jsr _13ec81
    }
}

// PITFALLS: 2 (Z/N flags must be set by caller to match `cmp #2` result)
// HELPERS: _13ec81_emu(snes)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x64=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::_13df4d ($DF:4D)