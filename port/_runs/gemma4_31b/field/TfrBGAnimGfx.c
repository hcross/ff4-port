#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$CB, DP=0
// This routine handles the transfer of background animation graphics to VRAM.
// It checks if the current background state matches specific animation flags
// before performing a DMA-like copy and updating VRAM registers.
static void TfrBGAnimGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Entry check: if (ram[0x1700] != 3) return
    if (ram[0x1700] != 3) return;

    // If (ram[0x7A] & 0x06 == 0) return
    if ((ram[0x7A] & 0x06) == 0) return;

    // Calculate VRAM offset based on flags in ram[0x7A]
    uint8_t flags = ram[0x7A];
    uint8_t mask_bits = (flags & 0x18);
    ram[0x12] = mask_bits;
    ram[0x13] = 0;

    // longa: Switch to 16-bit A
    // asl $12 x 4 shifts the value left by 4 bits
    uint16_t addr_val = read16(ram, 0x12);
    addr_val <<= 4; 
    
    // clc / adc #$5000
    addr_val += 0x5000;
    write16(ram, 0x12, addr_val);

    // shorta: Switch back to 8-bit A
    // Hardware register initialization
    ram[0x2115] = 0x80;
    ram[0x420B] = 0;
    ram[0x4300] = 0x01;
    ram[0x4301] = 0x18;
    
    // Note: Using write16 for 16-bit X registers
    write16(ram, 0x2116, 0x1200); 
    ram[0x4304] = 0x7F;
    write16(ram, 0x2116, 0x1200);

    uint8_t y = 4;
    do {
        // ldx $12 / stx $4302
        // Since X is 16-bit (xf=0), this loads the word from $12
        write16(ram, 0x4302, read16(ram, 0x12));
        
        // ldx #$0080 / stx $4305
        write16(ram, 0x4305, 0x0080);
        
        ram[0x420B] = 0x01;
        
        // lda $13 / clc / adc #$02 / sta $13
        ram[0x13] = (uint8_t)(ram[0x13] + 2);
        
        y--;
    } while (y != 0);
}

// PITFALLS: 6 (Mode A 8-bit vs 16-bit: handled via read16/write16 and explicit
// cast for the $13 increment), 7 (Arithmetic truncation: ram[0x13] cast to uint8_t)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1700=1, 0x7A=1
//   output_ram:  0x4302=2, 0x4305=2, 0x420B=1, 0x13=1, 0x2115=1, 0x2116=2, 0x4300=1, 0x4301=1, 0x4304=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xCB
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::TfrBGAnimGfx ($CB:5F)