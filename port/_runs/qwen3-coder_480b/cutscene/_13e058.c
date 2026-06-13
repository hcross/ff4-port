#include "snes/snes.h"

// This function configures and starts a DMA transfer to VRAM.
// It sets up the source address in $4342/$4344, the destination
// in hVMADDL, and initiates the transfer via hMDMAEN.
//
// Entry mode: A 16-bit (mf=0), X/Y 16-bit (xf=0), DB=0, DP=0
// Inputs:
//   A  = source bank (8-bit value, high byte ignored)
//   X  = source offset (16-bit)
//   Y  = VRAM destination address (16-bit)
//   $28 = transfer size (16-bit)
static void _13e058_c(Snes *snes, uint8_t src_bank, uint16_t src_offset, uint16_t vram_dst) {
    uint8_t *ram = snes->ram;
    // Simulate phb/pha/clr_a/pha/plb/pla sequence
    // This sequence effectively sets DB to 0 (since A is cleared and pulled to DB)
    snes->cpu->db = 0;

    // sty hVMADDL - Set VRAM destination address
    write16(ram, 0x2116, vram_dst); // hVMADDL is at $2116-$2117

    // stx $4342 - Set DMA source offset
    write16(ram, 0x4342, src_offset);

    // sta $4344 - Set DMA source bank
    ram[0x4344] = src_bank;

    // lda #$01 / sta $4340 - Set DMA transfer mode (1 register write once)
    ram[0x4340] = 0x01;

    // lda #<hVMDATAL / sta $4341 - Set DMA destination register (hVMDATAL)
    ram[0x4341] = 0x18; // Low byte of hVMDATAL ($2118)

    // ldx $28 / stx $4345 - Set transfer size
    uint16_t transfer_size = read16(ram, 0x28);
    write16(ram, 0x4345, transfer_size);

    // lda #$10 / sta hMDMAEN - Start DMA channel 4
    ram[0x420B] = 0x10; // hMDMAEN is at $420B
}

// PITFALLS: 1 (DB register manipulation via stack), 6 (A 16-bit mode assumed),
//           8 (A/X mode inherited from caller)
// HELPERS: read16/write16 for 16-bit memory access
// CONTRACT:
//   inputs_reg:  a=8, x=16, y=16
//   inputs_ram:  0x28=2
//   output_ram:  none
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::_13e058 ($E0:0058)