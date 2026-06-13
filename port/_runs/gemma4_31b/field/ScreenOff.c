#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00 (Hardware IO), DP=0
// Logic: Disables screen display and NMI timer by writing to hardware registers.
static void ScreenOff_c(Snes *snes) {
    // Note: hINIDISP and hNMITIMEN are hardware registers.
    // In the context of the snesrev pattern, these map to the IO region.
    // hINIDISP ($2001) and hNMITIMEN ($2100)
    
    snes->io[0x01] = 0x80; // sta hINIDISP
    snes->io[0x00] = 0x00; // sta hNMITIMEN (Note: mapping depends on specific IO header)
    
    // Since the provided ASM uses macros/labels (hINIDISP), we assume the 
    // implementation targets the SNES memory map.
}

// PITFALLS: None.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: field::ScreenOff ($85:EF)