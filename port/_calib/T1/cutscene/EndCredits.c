#include "snes/snes.h"

// Entry mode: Inherited from caller (A=?, X=?, DP=?, DB=?)
// Logic:
//   1. Sets CPU mode: X/Y to 16-bit, A to 8-bit.
//   2. Pushes PHP, PHB, PHD to stack (state preservation).
//   3. Sets cutscene ID to 2 and duration to 0x0A20 (2592 frames).
//   4. Branches to _d66b (likely the main cutscene loop or state update).
static void EndCredits_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // longi / shorta
    cpu->xf = false; // X/Y 16-bit
    cpu->mf = true;  // A 8-bit

    // php / phb / phd (State preservation)
    // Note: In a pure C reimplementation, these stack pushes are usually
    // implicit or irrelevant unless the delegated target reads the stack.
    // We simulate the registers if needed, but the target branch _d66b
    // typically handles the logic.
    
    // cutscene id: 2
    ram[0x0064] = 2;
    
    // cutscene duration: $0A20 (Little Endian)
    ram[0x006A] = 0x20;
    ram[0x006B] = 0x0A;

    // bra _d66b
    // This is a branch to another part of the same module.
    // Since we are in 'translate' mode and this is a jump to 
    // a label not provided as a separate function, we delegate 
    // the remaining execution from the branch target.
    run_emulated_func(snes, 0x00D66Bu);
}

// PITFALLS: None specifically triggered by logic, but observed mode 
// changes (longi/shorta) to ensure register sizes match expected state.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x0064=1, 0x006A=1, 0x006B=1
//   entry_mode:  mf=auto, xf=auto, dp=0x0, db=0x0
//   entry_flags: auto
// REVERSED_FUNCTION: cutscene::EndCredits ($D6:10)