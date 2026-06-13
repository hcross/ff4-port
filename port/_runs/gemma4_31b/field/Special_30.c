#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$17 (likely, based on stx $170c), DP=0
// This routine initializes the "Big Whale" event on the moon.
// It sets coordinates, triggers the whale boarding sequence, 
// marks the button state, and sets the whale's moon position.
static void Special_30_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // Set DB to $17 for the WRAM writes to $17xx
    cpu->db = 0x17;

    write16(ram, 0x170C, 0x2713);       // ldx #$2713 / stx $170c
    ram[0x1700] = 0x00;                 // lda #$00 / sta $1700
    
    // Travel to/from moon
    ram[0xC3] = 1;                      // lda #1 / sta $c3
    board_whale_emu(snes);              // jsr BoardWhale

    ram[0xA2] = 0xFF;                    // lda #$ff / sta $a2
    whale_button_emu(snes);             // jsr WhaleButton

    // Big whale is on the moon
    ram[0x1727] = 0x02;                 // lda #$02 / sta $1727
    write16(ram, 0x1725, 0x2713);       // ldx #$2713 / stx $1725

    // jmp WaitVblankEvent: The routine ends by jumping to a synchronization loop.
    // In the parity harness, we emulate the jump target.
    wait_vblank_event_emu(snes);
}

// PITFALLS: 1 (DB=$17 used for memory writes)
// HELPERS: board_whale_emu(snes), whale_button_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x170C=2, 0x1700=1, 0xC3=1, 0xA2=1, 0x1727=1, 0x1725=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x17
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::Special_30 ($C8:8B)