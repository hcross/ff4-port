#include "snes/snes.h"

// This is not a logic routine, but a definition of the 65816 Hardware Vector Table.
// The original ASM defines the jump targets for NMI, IRQ, and Reset.
// In the native C reimplementation, these are handled by the emulator's 
// interrupt logic rather than being called as a function.
static void Vectors_c(Snes *snes) {
    // This function is a placeholder for the memory layout at $FF:E0.
    // The hardware vectors are:
    // $FFEA: JmpNMI (NMI Vector)
    // $FFEE: JmpIRQ (IRQ Vector)
    // $FFFC: Reset   (Reset Vector)
    
    // No logic to execute.
}

// PITFALLS: None. This is a data table, not executable code.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFF
//   entry_flags: auto
// REVERSED_FUNCTION: field::Vectors ($FF:E0)