#include "snes/snes.h"

// Toggles the low bit of the event flag at $7E:00E3, then tail-jumps to
// WaitVblankEvent (delegated).  The original asm uses `jmp`, not `jsr`,
// so the emulated WaitVblankEvent will RTS directly to the caller of
// EventCmd_d0 — the parity harness sets up the SNES stack accordingly.
static void EventCmd_d0_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0xE3] ^= 1;                     // lda $e3 / eor #$01 / sta $e3
    wait_vblank_event_emu(snes);        // jmp WaitVblankEvent (tail call)
}

// PITFALLS: none (routine is trivial; no conditional branch at entry,
//          no mode changes, no hidden B issues)
// HELPERS: wait_vblank_event_emu(snes) — delegates WaitVblankEvent @ $EE:5B
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00E3=1
//   output_ram:  0x00E3=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none (routine does not read flags on entry)
// REVERSED_FUNCTION: field::EventCmd_d0 ($EE:1C)