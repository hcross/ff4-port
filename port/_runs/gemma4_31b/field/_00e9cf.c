#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$E9, DP=0
// This routine implements a timed delay/wait loop. It takes a timing 
// value in A, decomposes it to set a duration in $83 and a mask in $82,
// then loops until $81 reaches the value in $83 (modulo 32).
// It writes to hardware registers (0x212D-0x2132) during each frame.
static void _00e9cf_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = (uint8_t)snes->cpu->a;

    // pha / pha / and #$e0 / sta $06
    uint8_t high_mask = a & 0xE0;
    ram[0x06] = high_mask;

    // pla / and #$0f / asl / sec / adc $06 / sta $83
    uint8_t low_bits = (a & 0x0F);
    uint8_t shifted = (uint8_t)(low_bits << 1); // Pitfall 7
    uint8_t val_83 = (uint8_t)(shifted + high_mask + 1); // Pitfall 7 (sec / adc)
    ram[0x83] = val_83;

    // pla / and #$10 / beq @e9e5
    uint8_t flag_10 = a & 0x10;
    if (flag_10 == 0) {
        ram[0x82] = flag_10; // beq taken: sta $82 uses the result of 'and #$10' (0)
    } else {
        ram[0x82] = 0x07;     // beq not taken: lda #$07 / sta $82
    }

    ram[0x79] = 0; // stz $79
    ram[0x81] = 0; // stz $81

loop_e9eb:;
    WaitFrame_emu(snes);

    // Hardware register writes
    ram[0x212D] = 0; 
    ram[0x2131] = 0x83;
    ram[0x2132] = (ram[0x83] & 0xE0) | ram[0x81];

    ram[0x79]++; // inc $79
    if ((ram[0x79] & ram[0x82]) != 0) {
        goto loop_e9eb;
    }

    ram[0x81]++; // inc $81
    if ((ram[0x83] & 0x1F) != ram[0x81]) { // cmp $81 / bne @e9eb
        goto loop_e9eb;
    }

    ram[0x81]--; // dec $81
}

// PITFALLS: 7 (Arithmetic truncation for ASL and ADC in 8-bit mode), 
// 10 (Label loop_e9eb followed by statement)
// HELPERS: WaitFrame_emu(snes)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x81=1, 0x82=1, 0x83=1, 0x79=1, 0x2132=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE9
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00e9cf ($E9:CF)