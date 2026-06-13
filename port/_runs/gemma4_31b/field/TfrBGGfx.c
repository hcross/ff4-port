#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B1 (ROM), DP=0
// Logic:
// This routine performs a high-speed DMA transfer of BG graphics to VRAM.
// It iterates 384 times (@b151), each loop performing two MDMA bursts:
// 1. A burst with a 0x10 (16 byte) increment.
// 2. A burst with a 0x08 (8 byte) increment.
// This pattern suggests it is filling VRAM tiles by alternating offsets.
static void TfrBGGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x4301] = 0x18; // lda #$18 / sta $4301
    
    // hVMADDL is a symbol; based on the logic, it's likely a 16-bit 
    // VMA destination address register in WRAM/IO.
    write16(ram, 0x4302, 0x0000); // ldx #$0000 / stx hVMADDL (assuming 0x4302)
    
    uint16_t y = 0; // ldy #0
    do {
        // Burst 1
        ram[0x4306] = 0x80; // lda #$80 / sta hVMAINC (assuming 0x4306)
        ram[0x4300] = 0x01; // lda #$01 / sta $4300
        ram[0x4305] = 0x10; // ldx #$0010 / stx $4305
        ram[0x4307] = 0x01; // lda #$01 / sta hMDMAEN (assuming 0x4307)
        
        // Immediate clear of control registers to trigger/end DMA
        ram[0x4307] = 0x00; // stz hMDMAEN
        ram[0x4306] = 0x00; // stz hVMAINC
        ram[0x4300] = 0x00; // stz $4300

        // Burst 2
        ram[0x4305] = 0x08; // ldx #8 / stx $4305
        ram[0x4307] = 0x01; // lda #$01 / sta hMDMAEN
        
        y++; // iny
    } while (y != 0x0180); // cpy #$0180 / bne @b151
}

// PITFALLS: 1 (DB=$B1, though these are IO writes to $43xx range), 
// 6 (Sized mode: A is 8-bit, Y is 8-bit for the loop counter).
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x4300=1, 0x4301=1, 0x4305=1, 0x4307=1 (volatile IO)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB1
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::TfrBGGfx ($B1:43)