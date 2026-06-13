#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00 (Field module), DP=0
// This routine handles the "Wipe In" screen transition by waiting for 
// a specific number of Vblanks and updating hardware/RAM flags.
//
// Logic:
// 1. Initialize transition flags ($d9, $7a, $79) and hardware registers.
// 2. Loop: Wait for Vblank, then wait for the 2nd IRQ (ram[$7f] == 2).
// 3. Decrement counter ($79) until it underflows (bpl).
// 4. Finalize by clearing $d9 and setting hINIDISP based on ram[$b1].
static void WipeIn_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xD9] = 0x01;
    ram[0x7A] = 0x00;
    ram[0x79] = 0x1F;
    
    // Hardware register writes (mapped to snes->ram in this environment)
    ram[0x210B] = 0x80; // hINIDISP (Assuming mapping to 0x210B or similar)
    ram[0x2110] = 0x81; // hNMITIMEN (Assuming mapping to 0x2110 or similar)
    snes->cpu->c = true; // cli (Clear Interrupt disable flag)

    do {
        wait_vblank_long_emu(snes); // jsr WaitVblankLong

        // Wait for 2nd IRQ
        while (ram[0x7F] != 0x02) { // lda $7f / cmp #$02 / bne
            // Spin until IRQ counter reaches 2
        }

        // Pitfall 7: 8-bit decrement and BPL check
        // ram[0x79] is uint8_t; dec results in 0xFF (negative in 8-bit signed)
        ram[0x79]--; 
    } while ((int8_t)ram[0x79] >= 0); // bpl @927e

    ram[0xD9] = 0x00;

    uint8_t final_disp;
    if (ram[0xB1] == 0) { // lda $b1 / bne @9296
        final_disp = 0x0F; // lda #$0f
    } else {
        final_disp = ram[0x80]; // lda $80
    }
    
    ram[0x210B] = final_disp; // sta hINIDISP
}

// PITFALLS: 7 (8-bit signed comparison for bpl loop)
// HELPERS: wait_vblank_long_emu(snes) — delegates WaitVblankLong @ $91:2D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x7F=1, 0xB1=1, 0x80=1
//   output_ram:  0x210B=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::WipeIn ($92:69)