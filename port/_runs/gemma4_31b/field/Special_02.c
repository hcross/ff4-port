#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$D9, DP=0
// This routine manages a visual effect (Special_02) that iterates for 95 frames.
// It initializes coordinates/parameters and loops, calling a drawing routine
// and updating angle/radius counters.
static void Special_02_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xAD] = 0;            // stz $ad
    ram[0x06FB] = 0;          // ldx #0 / stx $06fb
    ram[0x24] = 0;            // stx $24
    ram[0x8F] = 0xC4;          // lda #$c4 / sta $8f
    ram[0x90] = 0x08;          // lda #$08 / sta $90
    ram[0x91] = 0x39;          // lda #$39 / sta $91

loop_d9af:
    wait_vblank_short_emu(snes); // jsr WaitVblankShort
    _00da21_emu(snes);            // jsr _00da21

    // longa: A becomes 16-bit
    uint16_t a16 = (uint16_t)ram[0x06FB]; 
    a16++;                       // inc (increment angle)
    ram[0x06FB] = (uint8_t)(a16 & 0xFF); // sta $06fb (stores low byte)

    uint16_t r16 = (uint16_t)ram[0x24];
    r16++;                       // inc (increment radius)
    ram[0x24] = (uint8_t)(r16 & 0xFF); // sta $24 (stores low byte)

    // lsr2 (lsr A twice in 16-bit mode)
    uint16_t res = r16 >> 2; 
    res &= 0x00FF;               // and #$00ff

    // shorta: A becomes 8-bit
    ram[0xAD] = (uint8_t)(res & 0xFF); // sta $ad

    if (ram[0xAD] == 0x5F) {     // cmp #$5f / beq @d9d3 (95 frames)
        wait_vblank_event_emu(snes); // jmp WaitVblankEvent
        return;
    }
    goto loop_d9af;              // jmp @d9af
}

// PITFALLS: 6 (Mode A switching: explicitly handled 16-bit increments and 
// shifts before truncating back to 8-bit for ram[0xAD] and counters).
// HELPERS: wait_vblank_short_emu, _00da21_emu, wait_vblank_event_emu
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0xAD=1, 0x24=1, 0x8F=1, 0x90=1, 0x91=1, 0x06FB=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xD9
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::Special_02 ($D9:99)