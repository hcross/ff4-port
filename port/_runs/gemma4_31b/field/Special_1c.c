#include "snes/snes.h"

// This routine marks the "enterprise" (likely a special entity/ship) as
// present on the overworld, sets its coordinates to (102, 158), 
// and triggers a VBlank wait event.
static void Special_1c_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x171F] = 0;                 // stz $171f
    ram[0xB7] = 0;                   // stz $b7 (Direct Page access assumed DP=0)
    
    write16(ram, 0x171D, 0x9E66);     // ldx #$9e66 / stx $171d
    
    ram[0x171C] = 0x01;              // lda #$01 / sta $171c
    
    wait_vblank_event_emu(snes);      // jmp WaitVblankEvent
}

// PITFALLS: None (straightforward linear sequence)
// HELPERS: wait_vblank_event_emu(snes) — delegates WaitVblankEvent @$E35B
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x171F=1, 0x171C=1, 0x171D=2, 0xB7=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: auto
// REVERSED_FUNCTION: field::Special_1c ($CF:D0)