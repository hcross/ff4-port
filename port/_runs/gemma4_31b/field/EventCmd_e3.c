#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00 (relative to $1000 range), DP=0
// Logic:
//   1. Reads a mask byte from the event stream.
//   2. Iterates through a buffer at $1003 (length 0x141).
//   3. For each byte, performs: buffer[x] = buffer[x] & mask; then calls NextChar.
//   4. Sets a flag at $CC and jumps to WaitVblankEvent.
static void EventCmd_e3_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // jsr GetNextEventByte
    uint8_t mask = get_next_event_byte_emu(snes);
    
    // sta $06
    ram[0x06] = mask;

    // ldx #0
    for (uint16_t x = 0; x <= 0x140; x++) {
        // lda $1003,x / and $06 / sta $1003,x
        // DP is 0, so $1003 is absolute. x is 16-bit.
        uint8_t val = ram[0x1003 + x];
        val &= ram[0x06];
        ram[0x1003 + x] = val;

        // jsr NextChar
        next_char_emu(snes);

        // cpx #$0140 / bne @ebb9
        // loop continues until x == 0x140
    }

    // lda #$01 / sta $cc
    ram[0xCC] = 0x01;

    // jmp WaitVblankEvent (Tail-call delegation)
    wait_vblank_event_emu(snes);
}

// PITFALLS: 6 (Mode A is 8-bit for the AND/STA operations), 8 (Inherited mf=true for 
// battle/event routines).
// HELPERS: get_next_event_byte_emu(snes), next_char_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0xCC=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_e3 ($EB:B1)