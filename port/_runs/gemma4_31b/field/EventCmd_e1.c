#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$EB, DP=0
// Logic:
//   1. Fetch next event byte.
//   2. Search for this byte in a table at $1440 (max 0x60 entries, word-stepped).
//   3. If found, clear the 2-byte entry at that location and wait for vblank.
//   4. If not found in first table, search in a table at $1033 (char-stepped).
//   5. If found in second table, clear the 2-byte entry at that location and wait for vblank.
//   6. If not found in either, call NextChar and continue searching second table.
//   7. If exhausted, jump to WaitVblankEvent.
static void EventCmd_e1_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    uint8_t event_byte = (uint8_t)get_next_event_byte_emu(snes);
    ram[0x06] = event_byte;

    // First table search: $1440,y (y += 2)
    for (uint16_t y = 0; y < 0x0060; y += 2) {
        if (ram[0x1440 + y] == event_byte) {
            ram[0x1440 + y] = 0;
            ram[0x1441 + y] = 0;
            wait_vblank_event_emu(snes);
            return;
        }
    }

    // Second table search: $1033,x
    for (uint16_t x = 0; x <= 0x0140; ) {
        if (ram[0x1033 + x] == event_byte) {
            ram[0x1033 + x] = 0;
            ram[0x1034 + x] = 0;
            wait_vblank_event_emu(snes);
            return;
        }
        
        next_char_emu(snes);
        x++; // cpx #$0140 / bne loop
    }

    wait_vblank_event_emu(snes);
}

// PITFALLS: None significant for this routine; standard 8-bit A / 16-bit Y iteration.
// HELPERS: get_next_event_byte_emu, next_char_emu, wait_vblank_event_emu
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xEB
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Multiple exit paths via WaitVblankEvent and RAM mutations)

// REVERSED_FUNCTION: field::EventCmd_e1 ($EB:53)