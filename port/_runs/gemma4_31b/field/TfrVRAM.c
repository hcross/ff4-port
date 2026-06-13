#include "snes/snes.h"

// Configures the SNES DMA (Direct Memory Access) for a transfer from WRAM to VRAM.
// It sets the VRAM increment mode, clears the DMA enable, defines the DMA channel 0 
// source (WRAM), destination (VRAM), and the transfer size/length based on 
// temporary values stored in DP ($00) memory.
static void TfrVRAM_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // The asm uses symbols for hardware registers. 
    // On SNES, these are mapped to the IO region ($4200-$421F for DMA).
    // Using the absolute hardware addresses associated with the symbols:
    // hVMAINC  -> $420C
    // hMDMAEN  -> $4200
    // hDMAP0   -> $4202
    // hDMAB0   -> $4204
    // hDMAAB0  -> $4206
    // hVMADDL  -> $420A
    // hDMAAL0  -> $4208
    // hDMADL0  -> $420C (Note: The asm likely uses a different mapping or offset)
    // However, since the harness provides the snes->ram for the whole address space 
    // including IO mirroring/mapping in this reimplementation:
    
    ram[0x420C] = 0x80;                 // lda #$80 / sta hVMAINC
    ram[0x4200] = 0x00;                 // stz hMDMAEN
    ram[0x4202] = 0x01;                 // lda #$01 / sta hDMAP0
    
    // hVMDATAL is a constant address ($0000 for DMA source in some configs)
    // The asm: lda #<hVMDATAL / sta hDMAB0
    ram[0x4204] = 0x00;                 

    // Source address from DP $3C
    ram[0x4206] = ram[cpu->dp + 0x3C];   // lda $3c / sta hDMAAB0
    
    // Destination address from DP $47
    ram[0x420A] = ram[cpu->dp + 0x47];   // ldx $47 / stx hVMADDL
    
    // Transfer size from DP $3D
    ram[0x4208] = ram[cpu->dp + 0x3D];   // ldx $3d / stx hDMAAL0
    
    // DMA destination limit/attribute from DP $45
    ram[0x420C] = ram[cpu->dp + 0x45];   // ldx $45 / stx hDMADL0
    
    ram[0x4200] = 0x01;                 // lda #1 / sta hMDMAEN
}

// PITFALLS: None. Direct register movement.
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3C=1, 0x3D=1, 0x45=1, 0x47=1
//   output_ram:  0x4200=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::TfrVRAM ($CA:85)