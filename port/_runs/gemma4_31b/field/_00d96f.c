#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=0, DP=0
// Logic: 
//   1. Store accumulator at offset $0300 + Y.
//   2. Read DP $23; if bit 0 is set, call SetSpriteMSB with A=0.
static void field_00d96f_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // sta $0300,y
    // Note: Y is 16-bit (xf=0). Offset is relative to DP=0.
    ram[0x0300 + cpu->y] = (uint8_t)cpu->a;

    // lda $23 / and #$01 / beq @d97e
    uint8_t flag = ram[0x23] & 0x01;
    if (flag == 0) {
        return;
    }

    // lda #0 / jsl SetSpriteMSB
    cpu->a = 0;
    set_sprite_msb_emu(snes);
}

// PITFALLS: None applicable for this routine's simple logic.
// HELPERS: set_sprite_msb_emu(snes) — delegates SetSpriteMSB @ $B3AF
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=16
//   inputs_ram:  0x23=1
//   output_ram:  0x0300+y=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00d96f ($D9:6F)