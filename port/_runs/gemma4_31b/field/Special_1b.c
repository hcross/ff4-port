#include "snes/snes.h"

// Entry mode: Inherited (assume mf=true, xf=false, dp=0, db=0xD0)
// This routine initializes two memory locations with specific offsets,
// calls a sub-routine, and then jumps to WaitVblankEvent.
// Note: The jump to WaitVblankEvent is a tail-call that does not return.
static void Special_1b_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // ldx #$0100 / stx $5e
    // X is 16-bit (xf=false). write16 is used for LE storage.
    write16(ram, 0x5E, 0x0100);

    // ldx #$01e0 / stx $60
    write16(ram, 0x60, 0x01E0);

    // jsr _00d02f
    _00d02f_emu(snes);

    // jmp WaitVblankEvent
    wait_vblank_event_emu(snes);
}

// PITFALLS: None. (Straight-line initialization and delegation).
// HELPERS: _00d02f_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x5E=2, 0x60=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xD0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: no
// REVERSED_FUNCTION: field::Special_1b ($D0:1F)