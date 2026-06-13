#include "snes/snes.h"

// Copies 256 bytes from $7E:0100 to VRAM at address $92, then updates $90/$92.
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: ram[$7D20] != 0 (checked by beq at start)
static void _13eb60_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if (ram[0x7D20] == 0) return; // beq @ebb7

    // Save and set DB to 0 (implicitly used by hardware register accesses)
    uint8_t old_db = snes->cpu->db;
    snes->cpu->db = 0;

    // Clear A (16-bit), push and set DB to 0
    snes->cpu->a = 0;
    // (No need to simulate stack ops for parity since no reads)

    // Set VMAINC for sequential writes
    ram[0x2115] = 0x00; // hVMAINC = $00

    // Set VMADDL from $92
    write16(ram, 0x2116, read16(ram, 0x92)); // hVMADDL = $92

    // Set up DMA: $4350-$4355
    write16(ram, 0x4352, read16(ram, 0x90)); // DMA source offset
    ram[0x4354] = 0x7E;                      // DMA source bank
    ram[0x4350] = 0x00;                      // DMA control
    ram[0x4351] = 0x18;                      // hVMDATAL (low byte of addr)
    write16(ram, 0x4355, 0x0100);            // DMA byte count

    // Start DMA
    ram[0x2119] = 0x20; // hMDMAEN = $20 (channel 5)

    // Set VMAINC for increment after high byte write
    ram[0x2115] = 0x80; // hVMAINC = $80

    // Restore DB
    snes->cpu->db = old_db;

    // Switch to 16-bit A
    snes->cpu->mf = false;

    // Update $90 and $92 by adding $0100
    uint16_t a90 = read16(ram, 0x90);
    write16(ram, 0x90, a90 + 0x0100); // clc / adc #$0100 / sta $90

    uint16_t a92 = read16(ram, 0x92);
    write16(ram, 0x92, a92 + 0x0100); // clc / adc #$0100 / sta $92

    // Switch back to 8-bit A
    snes->cpu->mf = true;

    // Check if $92 reached $4000
    if (read16(ram, 0x92) == 0x4000) { // cpx #$4000 / bne @ebb7
        ram[0x7D20] = 0;               // stz $7d20
    }
}

// PITFALLS: 1 (DB manipulation for hardware registers),
//           6 (A mode switching: 8-bit to 16-bit and back),
//           8 (A/X mode inheritance: assumed A 8-bit, X 16-bit at entry)
// HELPERS: none (no jsr targets)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x7d20=1, 0x90=2, 0x92=2
//   output_ram:  0x90=2, 0x92=2, 0x7d20=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::_13eb60 ($EB:0060)