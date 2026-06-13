#include "snes/snes.h"

// Performs a fade-out effect by gradually decreasing brightness and optionally
// switching screen settings. Waits for each frame during the fade.
static void _00ea14_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x79] = 0;                          // stz $79
loop_ea16:;
    wait_frame_emu(snes);                   // jsr WaitFrame
    uint8_t a = (ram[0x83] & 0xE0) | ram[0x81]; // and/or chain
    ram[0x2132] = a;                        // sta $2132
    ram[0x79]++;                            // inc $79
    a = ram[0x79] & ram[0x82];              // lda $79 / and $82
    if (a != 0) goto loop_ea16;             // bne @ea16
    ram[0x81]--;                            // dec $81
    if ((int8_t)ram[0x81] >= 0) goto loop_ea16; // bpl @ea16
    ram[0x81] = 0;                          // stz $81
    ram[0x212d] = 0x11;                     // lda #$11 / sta $212d
    if ((ram[0x0FE4] & 1) == 0) goto skip_ea48; // lsr / bcc @ea48
    ram[0x2130] = 0x02;                     // lda #$02 / sta $2130
    ram[0x2131] = 0x43;                     // lda #$43 / sta $2131
    return;                                 // jmp @ea4b / rts
skip_ea48:;
    ram[0x2131] = 0;                        // stz $2131
    // fallthrough to rts
}

// PITFALLS: 1 (DB=$7E assumed for WRAM access), 3 (BCC/BPL require signed
// comparison for proper emulation), 10 (goto label before declaration)
// HELPERS: wait_frame_emu(snes) — delegates WaitFrame @ $xx:8513
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x81=1, 0x82=1, 0x83=1, 0x0FE4=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none
// REVERSED_FUNCTION: field::_00ea14 ($EA:14)