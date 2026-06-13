#include "snes/snes.h"

// This function performs a DMA transfer to update video memory, then
// advances the source/destination pointers. It is called during cutscenes.
static void _13ebb8_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    if (ram[0x7D1F] != 0) goto skip_dma;     // bne @ec14
    if ((ram[0x4A] & 0x1F) != 0) goto skip_dma; // and #$1f / bne @ec14

    // Save and set data bank to 0
    uint8_t old_db = cpu->db;
    cpu->db = 0;

    // Set up DMA registers for transfer
    ram[0x2115] = 0x00;         // hVMAINC = 0
    write16(ram, 0x2116, read16(ram, 0x92)); // hVMADDL = $92
    write16(ram, 0x4352, read16(ram, 0x90)); // $4352 = $90
    ram[0x4354] = 0x7E;         // $4354 = #$7e
    ram[0x4350] = 0x00;         // $4350 = 0
    ram[0x4351] = 0x18;         // $4351 = #<hVMDATAL (0x2118)
    write16(ram, 0x4355, 0x0080); // $4355 = #$0080
    ram[0x420B] = 0x20;         // hMDMAEN = #$20

    // Set up for incrementing
    ram[0x2115] = 0x80;         // hVMAINC = #$80

    // Restore data bank
    cpu->db = old_db;

    // Switch to 16-bit A for pointer math
    cpu->mf = false;

    // Update $90 (source pointer)
    uint16_t src = read16(ram, 0x90);
    src = (uint16_t)(src + 0x0020);
    src &= 0x7FFF;
    src |= 0x8000;
    write16(ram, 0x90, src);

    // Update $92 (VRAM destination)
    uint16_t dst = read16(ram, 0x92);
    dst = (uint16_t)(dst + 0x0080);
    dst &= 0x3FFF;
    write16(ram, 0x92, dst);

    // Switch back to 8-bit A
    cpu->mf = true;

skip_dma:
    _13eb60_emu(snes);          // jsr _13eb60
}

// PITFALLS: 1 (DB register management), 6 (A register size changes),
//           8 (A/X mode inheritance - starts in 8-bit A, 16-bit X)
// HELPERS: _13eb60_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x7d1f=1, 0x4a=1, 0x90=2, 0x92=2
//   output_ram:  0x90=2, 0x92=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::_13ebb8 ($EB:B8)