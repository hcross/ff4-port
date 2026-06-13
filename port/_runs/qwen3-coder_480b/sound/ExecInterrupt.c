#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// All operations are on WRAM bytes at $00-$03 and hardware registers
// No input/output registers — all state in RAM
static void ExecInterrupt_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    // Transfer bytes from RAM $00-$03 to hardware registers hAPUIO0-hAPUIO3
    ram[0x2141] = ram[0x01];  // hAPUIO1
    ram[0x2142] = ram[0x02];  // hAPUIO2
    ram[0x2143] = ram[0x03];  // hAPUIO3
    ram[0x2140] = ram[0x00];  // hAPUIO0

    // Wait until hAPUIO0 matches the value just written
    while (ram[0x2140] != ram[0x00]) {
        // spin wait
    }

    // Clear hardware registers and RAM mirror
    ram[0x2140] = 0;  // hAPUIO0
    ram[0x2141] = 0;  // hAPUIO1
    ram[0x2142] = 0;  // hAPUIO2
    ram[0x2143] = 0;  // hAPUIO3
    ram[0x00] = 0;
    ram[0x01] = 0;
    ram[0x02] = 0;
    ram[0x03] = 0;
}

// PITFALLS: 1 (DB must be $7E to access WRAM), 6 (A is 8-bit due to shorta)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00=1, 0x01=1, 0x02=1, 0x03=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: sound::ExecInterrupt ($86:001E)