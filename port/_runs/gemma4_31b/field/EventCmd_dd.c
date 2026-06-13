#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$EC, DP=0
// Purpose: Scans a table at $1000 (relative to DP=0) for an event ID 
// match (lower 5 bits). Stores the index of the match and triggers a vblank wait.
static void EventCmd_dd_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // jsr GetNextEventByte
    uint8_t target_event = get_next_event_byte_emu(snes);
    ram[0x06] = target_event; // sta $06

    uint16_t x = 0; // ldx #0
    uint16_t y = 0; // ldy #0

loop_ec49:
    // lda $1000,y (DP=0, so absolute $1000 + y)
    uint8_t val = ram[0x1000 + y];
    val &= 0x1F; // and #$1f

    if (val == ram[0x06]) { // cmp $06 / beq @ec63
        ram[0x1703] = (uint8_t)x; // txa / sta $1703
        ram[0xCC] = 1;            // lda #1 / sta $cc
        wait_vblank_event_emu(snes); // jmp WaitVblankEvent
        return;
    }

    // longa / tya / clc / adc #$0040 / tay
    // This block increments the pointer Y by 0x40 (64 bytes)
    y += 0x40;

    // lda #0 / shorta / inx
    x++; 
    
    // jmp @ec49
    goto loop_ec49;
}

// PITFALLS: 1 (DB=$EC), 6 (Mode A transitions handled via local variables), 
// 8 (Inherited mf=true for battle/field event logic)
// HELPERS: get_next_event_byte_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1000=1 (table start)
//   output_ram:  0x1703=1, 0xCC=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xEC
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_dd ($EC:3E)