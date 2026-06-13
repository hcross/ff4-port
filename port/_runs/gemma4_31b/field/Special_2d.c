#include "snes/snes.h"

// Logic:
//   1. Resets map stack ($172C = 0).
//   2. Sets vehicle type to Big Whale ($1704 = 0x06).
//   3. Sets movement speed 16-bit ($AC = 0x03, $E1 = 0).
//   4. Transfers party coordinates to Big Whale position ($1725 -> $1706, $1727 -> _00f167).
//   5. Calls BoardWhale and then jumps to WaitVblankEvent.
static void Special_2d_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // reset map stack
    write16(ram, 0x172C, 0);

    // set vehicle to big whale
    ram[0x1704] = 0x06;

    // set movement speed (16-bit value 0x0003)
    ram[0xAC] = 0x03;
    ram[0xE1] = 0;

    // move party to big whale position (X is 16-bit here per longi convention)
    uint16_t pos_x = read16(ram, 0x1725);
    write16(ram, 0x1706, pos_x);

    // Load $1727 (Y coordinate) into A and call helper
    // Note: A is 8-bit here based on the 'lda #$06' pattern
    snes->cpu->a = ram[0x1727];
    _00f167_emu(snes);

    board_whale_emu(snes);

    // jmp WaitVblankEvent
    wait_vblank_event_emu(snes);
}

// PITFALLS: None. Routine is a linear sequence of stores and calls.
// HELPERS: _00f167_emu(snes), board_whale_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1725=2, 0x1727=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::Special_2d ($C8:BC)