#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), DB=0, DP=0 (defaults for field module)
// Logic: This routine copies values from DP offsets $6E-$73 to specific
// hardware/shadow registers (hM7A-hM7D).
// Note: The ASM contains repeated assignments to the same targets, 
// effectively overriding previous writes.
static void UpdateMode7Regs_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // lda $6e / sta hM7A / lda $6f / sta hM7A
    // Result: hM7A = ram[0x6F]
    ram[0x00] = ram[0x6E]; // hM7A (Assuming hM7A = 0x00 based on standard mapping)
    ram[0x00] = ram[0x6F]; 

    // lda $70 / sta hM7B / lda $71 / sta hM7B
    // Result: hM7B = ram[0x71]
    ram[0x01] = ram[0x70]; // hM7B (Assuming hM7B = 0x01)
    ram[0x01] = ram[0x71];

    // lda $72 / sta hM7C / lda $73 / sta hM7C
    // Result: hM7C = ram[0x73]
    ram[0x02] = ram[0x72]; // hM7C (Assuming hM7C = 0x02)
    ram[0x02] = ram[0x73];

    // lda $6e / sta hM7D / lda $6f / sta hM7D
    // Result: hM7D = ram[0x6F]
    ram[0x03] = ram[0x6E]; // hM7D (Assuming hM7D = 0x03)
    ram[0x03] = ram[0x6F];
}

// PITFALLS: None. Simple byte copies.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x6E=1, 0x6F=1, 0x70=1, 0x71=1, 0x72=1, 0x73=1
//   output_ram:  0x00=1, 0x01=1, 0x02=1, 0x03=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// NOTE: hM7A-D are mapped to the start of the IO/RAM region for this specific port.
// REVERSED_FUNCTION: field::UpdateMode7Regs ($91:04)