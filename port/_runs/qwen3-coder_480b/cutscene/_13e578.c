#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: X = offset into $2040/$20C0 tables
// Logic:
//   1. Push X
//   2. Switch to 16-bit A
//   3. Load $20C0,X → $18 (16-bit)
//   4. Load $2040,X → A (16-bit), then call _13e58b
//   5. Switch back to 8-bit A
//   6. Pull X and return
static void _13e578_c(Snes *snes, uint16_t x) {
    uint8_t *ram = snes->ram;
    // phx (X is already passed as argument)
    snes->cpu->mf = false;  // longa (A 16-bit)
    write16(ram, 0x18, read16(ram, 0x20C0 + x));  // lda $20c0,x / sta $18
    uint16_t a = read16(ram, 0x2040 + x);         // lda $2040,x
    snes->cpu->a = a;                             // prepare A for call
    _13e58b_emu(snes);                            // jsr _13e58b
    snes->cpu->mf = true;                         // shorta0 (A 8-bit)
    // plx / rts (return, X restored by caller context)
}

// PITFALLS: 1 (DB=$7E assumed), 8 (mode A/X inheritance — routine starts
// without explicit mode directive, so we infer from context: A=8, X=16)
// HELPERS: _13e58b_emu(snes) — delegates _13e58b @ $E5:8B
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  0x2040=2, 0x20C0=2
//   output_ram:  0x18=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::_13e578 ($E5:78)