#include "snes/snes.h"

// Toggles bit 0 of $E3, then tail-calls WaitVblankEvent.
// Entry mode: A 8-bit (mf=1), inherited from field module convention.
static void EventCmd_d0_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t val = ram[0xE3] ^ 0x01;
    ram[0xE3] = val;
    // jmp passes A and flags through to WaitVblankEvent
    Cpu *c = snes->cpu;
    c->a = val;
    c->z = (val == 0);
    c->n = (val & 0x80) != 0;
    wait_vblank_event_emu(snes);
}

// PITFALLS: 2 (A and Z/N flags passed through jmp to WaitVblankEvent —
//   must be set before emu call so the delegated function sees correct
//   entry state)
// HELPERS: wait_vblank_event_emu(snes) — delegates WaitVblankEvent @ $E35B
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xE3=1
//   output_ram:  0xE3=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_d0 ($EE:1C)