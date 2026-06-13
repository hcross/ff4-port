#include "snes/snes.h"

// Logic:
//   1. Set the value 0x10 in RAM address $A2.
//   2. Execute the LandEnterprise routine.
//   3. Transfer execution to WaitVblankEvent (which is a terminal jump).
static void Special_15_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xA2] = 0x10;              // lda #$10 / sta $a2
    land_enterprise_emu(snes);      // jsr LandEnterprise
    wait_vblank_event_emu(snes);    // jmp WaitVblankEvent
}

// PITFALLS: None relevant. Routine performs simple immediate store and delegation.
// HELPERS: 
//   land_enterprise_emu(snes) - delegates LandEnterprise @$A60B
//   wait_vblank_event_emu(snes) - delegates WaitVblankEvent @$E35B
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x00A2=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::Special_15 ($DC:04)