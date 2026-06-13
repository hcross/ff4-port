#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00 (HW regs), DP=0
// Logic: Restores system state after a cutscene by re-initializing interrupts,
// hardware registers, enabling NMI, and turning the screen on.
static void AfterCutscene_c(Snes *snes) {
    // jsr InitInterrupts
    InitInterrupts_emu(snes);
    
    // jsl InitHWRegs
    InitHWRegs_emu(snes);
    
    // cli
    snes->cpu->i = false;
    
    // lda #$00 / sta $2100 (Screen on, zero brightness)
    snes->ram[0x2100] = 0x00;
    
    // lda #$81 / sta $4200 (Enable NMI)
    snes->ram[0x4200] = 0x81;
}

// PITFALLS: None
// HELPERS: InitInterrupts_emu(snes), InitHWRegs_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x2100=1, 0x4200=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: auto
// REVERSED_FUNCTION: field::AfterCutscene ($C5:9A)