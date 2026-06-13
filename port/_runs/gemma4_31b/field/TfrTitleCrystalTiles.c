#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00, DP=0
// Purpose: Transfers a burst of 8 bytes (4 pairs) of tile data from ROM to VRAM 
// via the VMA registers, using $47 as the base address and $3D as the offset.
static void TfrTitleCrystalTiles_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // Writing to hardware VMA registers (mapped to I/O space)
    // The previous attempt failed because hVMAINC etc are not C variables.
    // In the target environment, these are written via the VRAM API.
    vram_set_vma_inc(0x80);              // sta hVMAINC
    vram_set_vma_addr(ram[0x47]);        // ldx $47 / stx hVMADDL
    
    uint16_t x_reg = ram[0x3D];          // ldx $3d
    
    // The loop iterates while (x & 7) != 0.
    // It processes 2 bytes per iteration (pair of bytes).
    // ROM base is determined by TitleCrystalTiles location in bank $88.
    uint32_t rom_base = 0x880000 + 0x0000; // .bankbyte(TitleCrystalTiles)<<16

    while (1) {
        // lda rom,x / sta hVMDATAL
        vram_set_vma_data_lo(snes->rom[rom_base + x_reg]);
        // lda rom,x+1 / sta hVMDATAH
        vram_set_vma_data_hi(snes->rom[rom_base + x_reg + 1]);
        
        x_reg += 2;                      // inx2
        if ((x_reg & 7) == 0) break;     // txa / and #7 / bne @886a
    }

    // longa : A becomes 16-bit
    cpu->mf = false; 
    
    uint16_t a_val = ram[0x3D];          // lda $3d
    a_val += 8;                          // clc / adc #8
    ram[0x3D] = (uint8_t)a_val;          // sta $3d (writes low byte)

    // tdc : A = DP (which is 0)
    cpu->a = cpu->dp; 
    
    // xba : Swap A and X. A was 0, X was the loop end value.
    uint16_t temp = cpu->a;
    cpu->a = cpu->x;
    cpu->x = temp;

    // shorta : A becomes 8-bit
    cpu->mf = true;
    
    // iny2 : Y += 2
    cpu->y += 2;
}

// PITFALLS: 6 (Mode A transitions between 8-bit and 16-bit), 5 (tdc acts as clr_a since DP=0)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x47=1, 0x3D=1
//   output_ram:  0x3D=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::TfrTitleCrystalTiles ($88:5E)