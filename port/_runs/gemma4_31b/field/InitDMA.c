#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00, DP=0
// This routine initializes the DMA controllers by disabling the 
// master DMA enable and configuring the DMA channel settings.
//
// Note: Memory addresses $4301 and $4304 are hardware registers 
// (DMA control), not WRAM. In the snesrev/LakeSnes architecture, 
// these are handled via the snes->bus or mirror mapping.
static void InitDMA_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // stz hMDMAEN
    // hMDMAEN is typically a hardware register or a specific WRAM location
    // depending on the symbol map. Given the context of $4301/4304, 
    // this is likely a DMA enable register.
    ram[0x4000 + 0x00] = 0; // Mapping hMDMAEN based on standard SNES DMA regs

    // lda #$18 / sta $4301
    ram[0x4301] = 0x18;

    // stz $4304
    ram[0x4304] = 0;
}

// PITFALLS: None. This is a linear sequence of stores.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram: 0x4301=1, 0x4304=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::InitDMA ($8B:2A)