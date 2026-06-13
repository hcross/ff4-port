#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x0, DP=0
// Logic: Enables MDMA by writing 1 to the hMDMAEN hardware register.
static void ExecDMA_c(Snes *snes) {
    // hMDMAEN is a hardware register. 
    // In the context of this project, these are accessed via snes->ram 
    // mapped to the SNES I/O space or specific hardware offsets.
    // hMDMAEN corresponds to $00:0001 (or specifically the MDMA enable bit).
    snes->ram[0x0001] = 1; 
}

// PITFALLS: None. Simple immediate load and store to hardware register.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x0001=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::ExecDMA ($8B:36)