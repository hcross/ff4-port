#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$80, DP=0
// Logic:
//   1. Resets hardware registers.
//   2. Updates player movement speed.
//   3. Sets VBlank wait flag (ram[0x7D]).
//   4. Clears dialogue window height (ram[0xDF]) and event flag (ram[0xB1]).
//   5. Jumps to LoadMap.
static void AfterBattle_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    InitHWRegs_emu(snes);           // jsl InitHWRegs
    UpdatePlayerSpeed_emu(snes);    // jsr UpdatePlayerSpeed

    ram[0x7D] = 1;                  // lda #1 / sta $7d
    ram[0xDF] = 0;                  // stz $df
    ram[0xB1] = 0;                  // stz $b1

    LoadMap_emu(snes);              // jmp LoadMap
}

// PITFALLS: None.
// HELPERS: InitHWRegs_emu(snes), UpdatePlayerSpeed_emu(snes), LoadMap_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x7D=1, 0xDF=1, 0xB1=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x80
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::AfterBattle ($80:8E)