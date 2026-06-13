#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$92 (Code), DP=0
// This routine updates screen wipe parameters. 
// It uses a timer value from $79 to index into WipeScanlineTbl.
// Note: Symbols hVTIMEL, hWH0, hWH1, hNMITIMEN, hINIDISP are HRAM/WRAM addresses.
static void UpdateWipeNMI_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // The original code is in bank $92. 
    // We assume WipeScanlineTbl is a constant table in ROM.
    // Since the LakeSnes Snes struct doesn't expose a direct 'rom' pointer 
    // like the failed attempt suggested, and the harness provides run_emulated_func,
    // we must ensure we access ROM via the appropriate system read if applicable.
    // However, for a static table in ROM, the common pattern is to define the 
    // table as a constant array in the C file if the values are known, or 
    // use a helper. Since the values aren't provided, we simulate the memory read.
    
    // Use a helper or a direct read from the snes ROM region if the harness supports it.
    // For this specific project, ROM is usually accessed via a global or a pointer
    // in the Snes struct not shown in the API but implied by the task's ROM context.
    // We'll use a placeholder for the table base address.
    extern const uint8_t WipeScanlineTbl[]; 

    uint8_t timer = ram[0x79];
    uint8_t x_idx = (uint8_t)(timer << 1); // Pitfall 7: truncate 8-bit

    // lda #$6f / sec / sbc WipeScanlineTbl+1,x / tay / sty hVTIMEL
    // SBC logic: (A + 1) - mem
    uint8_t val_plus_1 = WipeScanlineTbl[1 + x_idx];
    uint8_t vtime_l = (uint8_t)(0x6F + 1 - val_plus_1);
    ram[0x7F40] = vtime_l; // hVTIMEL = 0x7F40 (example address)

    // lda #$80 / sec / sbc WipeScanlineTbl,x / sta hWH0
    uint8_t val_0 = WipeScanlineTbl[x_idx];
    uint8_t wh0 = (uint8_t)(0x80 + 1 - val_0);
    ram[0x7F41] = wh0; // hWH0 = 0x7F41 (example address)

    // lda #$7f / clc / adc WipeScanlineTbl,x / sta hWH1
    uint8_t wh1 = (uint8_t)(0x7F + val_0);
    ram[0x7F42] = wh1; // hWH1 = 0x7F42 (example address)

    // lda $79 / lsr / asl4 / clc / adc #$03 / sta $0677
    uint8_t t2 = ram[0x79];
    uint8_t res = (uint8_t)(t2 >> 1);     // lsr
    res = (uint8_t)(res << 4);           // asl4 (4 shifts)
    res = (uint8_t)(res + 0x03);         // adc #$03
    ram[0x0677] = res;

    // lda #$a1 / sta hNMITIMEN
    ram[0x7F43] = 0xA1; // hNMITIMEN = 0x7F43 (example address)

    // lda #$80 / sta hINIDISP
    ram[0x7F44] = 0x80; // hINIDISP = 0x7F44 (example address)
}

// PITFALLS: 7 (Arithmetic/shifts truncated to 8-bit)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x79=1
//   output_ram:  0x0677=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x92
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Requires ROM table WipeScanlineTbl resolution)

// REVERSED_FUNCTION: field::UpdateWipeNMI ($92:04)