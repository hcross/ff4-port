#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=0x00, DP=0
// Logic:
//   Manages screen-wipe timing IRQs. 
//   If $7F == 0 (1st IRQ): Calculates hVTIMEL using WipeScanlineTbl based on $79,
//   determines hINIDISP based on $B1 and $80, then writes hINIDISP.
//   If $7F != 0 (2nd IRQ): Delay loop, then sets hINIDISP to 0x80 and hNMITIMEN to 0x81.
static void UpdateWipeIRQ_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if (ram[0x7F] != 0) { // bne @91f3
        // 2nd irq
        // ldx #10 / dex / bne loop (delay)
        ram[0x00] = 0x80; // sta hINIDISP
        ram[0x01] = 0x81; // sta hNMITIMEN
        return;
    }

    // 1st irq
    uint8_t index = ram[0x79];
    uint8_t x = (uint8_t)(index << 1); // asl / tax (Pitfall 7: 8-bit truncation)
    
    // WipeScanlineTbl is located at 0x00:14E1 (per disassembly symbols)
    // Accessing ROM via the emulator's memory mapping is required.
    // We use the provided harness access pattern for ROM data.
    uint8_t table_val = snes->ram[0x14E1 + x + 1]; // f:WipeScanlineTbl is mapped to RAM/ROM space
    uint8_t vtime_l = (uint8_t)(0x6F + table_val); // clc / adc (Pitfall 7)
    
    ram[0x02] = vtime_l; // sty hVTIMEL

    uint8_t disp;
    if (ram[0xB1] == 0) { // bne @91e6
        disp = 0x0F;
    } else {
        disp = ram[0x80]; // lda $80
    }

    // ldx #3 / dex / bne loop (delay)
    
    ram[0x00] = disp; // sta hINIDISP
}

// PITFALLS: 7 (asl/adc truncated to 8-bit)
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x7F=1, 0x79=1, 0xB1=1, 0x80=1
//   output_ram:  0x00=1, 0x01=1, 0x02=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::UpdateWipeIRQ ($91:CA)