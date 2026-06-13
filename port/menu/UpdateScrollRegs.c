#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$FF, DP=0
// The routine synchronizes software scroll registers (stored in DP relative memory)
// to the hardware scroll registers.
// 
// Logic:
// Copies 8-bit values from DP offsets 0x8A-0xA0 into the corresponding 
// hardware register mapped names. Each hardware register is written to 
// twice (low then high, though the indices in the ASM suggest 
// sequence of individual byte writes to the same destination label).
static void UpdateScrollRegs_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // The ASM performs pha/phb/tdc/pha/plb sequence which preserves stack/B
    // and clears A via tdc. This has no side effect on RAM.

    // BG2 HOFS
    ram[hBG2HOFS] = ram[0x8A];
    ram[hBG2HOFS] = ram[0x8B]; // Overwrites previous write (matches ASM sequence)

    // BG2 VOFS
    ram[hBG2VOFS] = ram[0x8D];
    ram[hBG2VOFS] = ram[0x8E];

    // BG1 HOFS
    ram[hBG1HOFS] = ram[0x90];
    ram[hBG1HOFS] = ram[0x91];

    // BG1 VOFS
    ram[hBG1VOFS] = ram[0x93];
    ram[hBG1VOFS] = ram[0x94];

    // BG4 HOFS
    ram[hBG4HOFS] = ram[0x96];
    ram[hBG4HOFS] = ram[0x97];

    // BG4 VOFS
    ram[hBG4VOFS] = ram[0x99];
    ram[hBG4VOFS] = ram[0x9A];

    // BG3 HOFS
    ram[hBG3HOFS] = ram[0x9C];
    ram[hBG3HOFS] = ram[0x9D];

    // BG3 VOFS
    ram[hBG3VOFS] = ram[0x9F];
    ram[hBG3VOFS] = ram[0xA0];
}

// PITFALLS: None. Simple byte transfers. Note that the ASM 
// writes twice to the same label (e.g. sta hBG2HOFS), implying 
// these labels likely refer to the same address or the logic 
// intentionally overwrites.
// HELPERS: None.

// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x8A=1, 0x8B=1, 0x8D=1, 0x8E=1, 0x90=1, 0x91=1, 0x93=1, 0x94=1, 0x96=1, 0x97=1, 0x99=1, 0x9A=1, 0x9C=1, 0x9D=1, 0x9F=1, 0xA0=1
//   output_ram:  none (writes to hardware/global labels)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFF
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (writes to multiple diverse hardware labels)

// REVERSED_FUNCTION: menu::UpdateScrollRegs ($FF:0A)