#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Inputs:  ram[$00] to ram[$03] contain the sound effect data
// Outputs: writes to hAPUIO0-hAPUIO3 (hardware-mapped I/O), then clears them
//          and the source bytes
static void PlayGameSfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    // shorta is already in effect (A is 8-bit)
    ram[0x2143] = ram[0x03];  // hAPUIO3
    ram[0x2142] = ram[0x02];  // hAPUIO2
    ram[0x2141] = ram[0x01];  // hAPUIO1
    ram[0x2140] = ram[0x00];  // hAPUIO0

    // Poll until hAPUIO0 matches the written value
    while (ram[0x2140] != ram[0x00]) {
        // spin wait (bne @85f7)
    }

    // Clear registers
    ram[0x2140] = 0;  // hAPUIO0
    ram[0x00] = 0;
    ram[0x01] = 0;
    ram[0x02] = 0;
    ram[0x03] = 0;
}

// PITFALLS: 1 (DB must be $7E for hardware register access),
//           7 (hAPUIO* are at fixed addresses 0x2140-0x2143)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00=1, 0x01=1, 0x02=1, 0x03=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: sound::PlayGameSfx ($85:E1)