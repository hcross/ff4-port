#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0 (Hardware/System), DP=0
// This routine performs a hardware-level wipe/reset of timers and interrupts,
// then polls a specific RAM location ($7F) to synchronize with two Vblank interrupts
// while counting up to 0x20 iterations in RAM $79.
static void WipeOut_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Initial hardware/state setup
    ram[0xD9] = 0x01;
    ram[0x00] = 0x07; // hTM (Assume DP=0, mapping to 0x00)
    ram[0x01] = 0x01; // hTS (Assume DP=0, mapping to 0x01)
    ram[0x7A] = 0;
    ram[0x79] = 0;
    ram[0x02] = 0x81; // hNMITIMEN (Assume DP=0, mapping to 0x02)
    
    snes->cpu->i = false; // cli

    do {
        wait_vblank_long_emu(snes); // jsr WaitVblankLong

        // Wait for 2nd IRQ signal in $7F
        while (ram[0x7F] != 0x02) { // lda $7f / cmp #$02 / bne @925a
            // Spin lock
        }

        ram[0x79]++; // inc $79
        if (ram[0x79] == 0x20) { // lda $79 / cmp #$20 / bne @9257
            break;
        }
    } while (true);
}

// PITFALLS: 1 (Sectors $00-$02 are treated as hardware/DP offsets here, 
// DB is effectively 0 for system registers)
// HELPERS: wait_vblank_long_emu(snes) — delegates WaitVblankLong @$91:2D
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x7F=1
//   output_ram:  0x79=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::WipeOut ($92:3F)