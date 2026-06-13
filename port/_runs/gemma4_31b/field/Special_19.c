#include "snes/snes.h"

// Logic:
//   Sets hardware/system registers and local state (WRAM) to initialize a 
//   specific game state (likely a menu or a special dialogue sequence), 
//   then calls a routine at 0x00D02F and jumps to the Vblank waiter.
static void Special_19_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda #$16 / sta $212c
    // Note: $212C is typically an I/O register (likely H-Blank/V-Blank/Screen)
    // In this harness, we write directly to ram if mapped, but $21xx 
    // is usually handled via snes->io or a specific memory map.
    // Following the provided pattern for ram access:
    ram[0x212C] = 0x16;

    // lda #$01 / sta $c9
    ram[0xC9] = 0x01;

    // ldx #$0000 / stx $5e
    write16(ram, 0x5E, 0x0000);

    // ldx #$02e0 / stx $60
    write16(ram, 0x60, 0x02E0);

    // jsr _00d02f
    _00d02f_emu(snes);

    // jmp WaitVblankEvent
    // Jump translates to a function call in C, as it doesn't return here.
    wait_vblank_event_emu(snes);
}

// PITFALLS: 1 (Standard memory mapping assumed for $212C in this context)
// HELPERS: _00d02f_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x212C=1, 0x00C9=1, 0x005E=2, 0x0060=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::Special_19 ($CF:F6)