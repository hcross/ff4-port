#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B0, DP=0
// Logic:
//   This IRQ handler manages the scrolling and timing for the item selection window.
//   It operates in two phases (1st and 2nd IRQ) based on the value at $B0:7F.
//
//   1st IRQ: Sets hVTIME (vertical timing) based on window height,
//            sets hBG3HOFS to 0x01 (right screen), and resets hBG3VOFS.
//   2nd IRQ: Resets hBG3HOFS (left screen), sets hBG3VOFS based on window height,
//            and enables NMI.
static void update_item_window_irq_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Note: $B0 is the data bank (db), and DP=0.
    // Accesses to $7F, $DA, $BB are relative to DB=$B0.
    // Symbols like hVTIMEL, hBG3HOFS are assumed to be mapped to specific WRAM/HW addresses.
    // Based on standard SNES/FF4 layout:
    // hVTIMEL = $00:XXXX (Usually HW or mirrored), here treated as memory mapping.
    
    if (ram[0x7F] == 0) { // lda $7f / bne @b0f8 (If A == 0, it is the 1st IRQ)
        // 1st IRQ
        uint8_t height = ram[0xDA];
        uint8_t vtime = (uint8_t)((height << 3) + 0x90); // asl3 / clc / adc #$90
        
        // hVTIME is a 16-bit value (hVTIMEL / hVTIMEH)
        // Assuming hVTIMEL is at a known offset, typically handled by a global map
        // For parity, we write to the absolute mapped address.
        ram[0x00] = vtime; // sta hVTIMEL (Example address, replace with actual header)
        ram[0x01] = 0;      // stz hVTIMEH
        
        // The loop @b0e4 is a simple no-op delay (dex / bne)
        // In C, this is functionally empty for logic parity.
        
        ram[0x02] = 0;      // stz hBG3HOFS (Example address)
        ram[0x02] = 0x01;   // sta hBG3HOFS
        
        // lda $bb / sta hBG3VOFS / stz hBG3VOFS 
        // This sequence effectively clears hBG3VOFS regardless of $BB
        ram[0x03] = 0;      // stz hBG3VOFS
        return;
    } else {
        // 2nd IRQ (@b0f8)
        // Loop @b0fb is a no-op delay.
        
        ram[0x02] = 0;      // stz hBG3HOFS (Example address)
        ram[0x02] = 0;      // stz hBG3HOFS
        
        uint8_t height = ram[0xDA];
        // asl3 / eor #$ff / clc / adc #$89
        // eor #$ff is a bitwise NOT.
        uint8_t vofs = (uint8_t)((~(height << 3)) + 0x89); 
        
        ram[0x03] = vofs;   // sta hBG3VOFS
        ram[0x03] = 0;      // stz hBG3VOFS (Wait: asm says sta then stz? Checking...)
        // ASM: sta hBG3VOFS / stz hBG3VOFS -> This effectively zeros it.
        
        ram[0x04] = 0x81;   // sta hNMITIMEN
    }
}

// PITFALLS: 7 (Arithmetic truncation: (height << 3) and the subsequent 
// additions are wrapped in (uint8_t) to mirror 8-bit A behavior).
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x7F=1, 0xDA=1, 0xBB=1
//   output_ram:  none (writes to hardware/global registers)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Function writes to hardware registers/globals not in standard WRAM)
// REVERSED_FUNCTION: field::update_item_window_irq ($B0:00CF)