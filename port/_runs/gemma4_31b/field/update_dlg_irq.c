#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x0 (Hardware/Mirror), DP=0
// This routine handles the timing and scrolling for dialogue windows via IRQs.
// It uses hardware register addresses (DP=0) for V-blank/H-blank timing and scrolling.
// Note: hVTIMEL, hBG3HOFS, etc., map to the SNES hardware registers in snes->ram 
// or the emulator's memory map.
static void update_dlg_irq_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // lda $7f
    uint8_t irq_state = ram[0x7F];
    
    if (irq_state != 0) { // bne @b73d
        // 2nd irq: Close window / Scroll left
        // ldx #8 / dex / bne @b740 (Delay loop)
        
        // stz hBG3HOFS / stz hBG3HOFS (Clear horizontal scroll)
        ram[0x00A0] = 0; // hBG3HOFS (Assuming standard mirror map)
        ram[0x00A1] = 0; 
        
        // lda $df (dialogue window height)
        uint8_t height = ram[0xDF];
        
        // asl3 (multiply by 8)
        uint8_t val = (uint8_t)(height << 3); // Pitfall 7
        
        // eor #$ff / clc / adc #5
        val = (uint8_t)((val ^ 0xFF) + 5); // Pitfall 7
        
        // sta hBG3VOFS / stz hBG3HOFS (Writing 16-bit value as low then high)
        ram[0x00A2] = val; // hBG3VOFS
        ram[0x00A3] = 0;
        
        // lda #$81 / sta hNMITIMEN
        ram[0x00C0] = 0x81; // hNMITIMEN (Enable NMI/Disable IRQ)
        return;
    }

    // 1st irq: Open window / Scroll right
    // lda $df (dialogue window height)
    uint8_t height = ram[0xDF];
    
    // asl3 (multiply by 8)
    uint8_t val = (uint8_t)(height << 3); // Pitfall 7
    
    // clc / adc #$14
    val = (uint8_t)(val + 0x14); // Pitfall 7
    
    // sta hVTIMEL / stz hVTIMEH
    ram[0x00C2] = val; // hVTIMEL
    ram[0x00C3] = 0;   // hVTIMEH
    
    // ldx #8 / dex / bne @b729 (Delay loop)
    
    // stz hBG3HOFS / lda #1 / sta hBG3HOFS (Scroll to right screen)
    ram[0x00A0] = 0;
    ram[0x00A1] = 1; // Setting the second byte of HFS
    
    // lda $bb / sta hBG3VOFS / stz hBG3VOFS
    uint8_t scroll_pos = ram[0xBB];
    ram[0x00A2] = scroll_pos; // hBG3VOFS
    ram[0x00A3] = 0;
}

// PITFALLS: 7 (8-bit arithmetic truncation for asl3 and adc)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x7F=1, 0xDF=1, 0xBB=1
//   output_ram:  0x00C2=1, 0x00A0=1, 0x00A2=1, 0x00C0=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::update_dlg_irq ($B7:14)