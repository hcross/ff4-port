#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00, DP=0
// This routine sets battle ID flags, selects the background, 
// and optionally disables magnetization based on event switch 0xE1.
static void UpdateBattleParams_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    if (ram[0x1701] != 0) {      // lda $1701 / beq @8cbc
        ram[0x1801] = 0x01;      // lda #$01
    } else {
        ram[0x1801] = 0;         // beq @8cbc / sta $1801
    }

    GetBattleBG_emu(snes);       // jsr GetBattleBG

    cpu->a = 0xE1;               // lda #$e1
    CheckEventSwitch_emu(snes);  // jsr CheckEventSwitch
    
    // CheckEventSwitch returns the result in A.
    // cmp #$00 / bne @8cd3
    if (cpu->a == 0) {
        ram[0x1802] &= 0x7F;     // lda $1802 / and #$7f / sta $1802
    }
}

// PITFALLS: 7 (Arithmetic/Logic truncation: explicitly using uint8_t for RAM 
// accesses and 8-bit AND operation).
// HELPERS: GetBattleBG_emu(snes), CheckEventSwitch_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1701=1, 0x1802=1
//   output_ram:  0x1801=1, 0x1802=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::UpdateBattleParams ($8C:B5)