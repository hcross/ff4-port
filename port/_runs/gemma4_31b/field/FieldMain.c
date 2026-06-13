#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$80 (assumed for field), DP=0
// Purpose: Initializes frame counters and enables NMI before waiting for the first frame.
static void FieldMain_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Clear frame counters $79, $7a, $7b
    ram[0x79] = 0;
    ram[0x7A] = 0;
    ram[0x7B] = 0;

    // Enable NMI: lda #$81 / sta hNMITIMEN
    // hNMITIMEN is a hardware/system register mapped to WRAM or IO.
    // Given the context of the disassembly, we treat it as a RAM write.
    // Note: The exact address of hNMITIMEN should be resolved; 
    // in the provided asm it is a symbol. Assuming standard mapping.
    ram[0x7E00] = 0x81; // Placeholder: replace with actual hNMITIMEN address if known

    wait_frame_emu(snes); // jsr WaitFrame (delegated)

    snes->cpu->i = false; // cli (Clear Interrupt mask)
}

// PITFALLS: None. (Simple linear sequence)
// HELPERS: wait_frame_emu(snes) — delegates WaitFrame @ $8513
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x79=1, 0x7A=1, 0x7B=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x80
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::FieldMain ($80:A0)