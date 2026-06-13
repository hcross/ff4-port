#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=0x0, DP=0
// Logic:
//   If (ram[0x7A] & 0x03) != 0, return immediately.
//   Otherwise, increment the palette index in ram[0x79], wrapping at 6.
//   Calculate a source offset (ram[0x79] * 2).
//   Copy 20 pairs of bytes (40 bytes total) from the whirlpool palette table 
//   starting at $0AAF + offset to the active palette ram starting at $0E1D.
static void ShiftWhirlpoolPal_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if ((ram[0x7A] & 0x03) != 0) { // and #$03 / bne @d341
        return;
    }

    ram[0x79]++; // inc $79
    if (ram[0x79] == 6) { // cmp #$06 / bne @d31a
        ram[0x79] = 0; // stz $79
    }

    uint8_t pal_idx = ram[0x79];
    uint8_t offset = (uint8_t)(pal_idx << 1); // asl A (Pitfall 7: 8-bit truncate)
    
    uint16_t x = 0; // ldx #0
    uint16_t y = offset; // tay

    // Loop copies 20 pairs of bytes. 
    // The ASM uses a nested logic with a jump back to @d321.
    // It iterates x from 0 to 14 (step 2) and y from offset to offset+28 (step 2).
    while (x < 20) { // cpx #14 is likely for 10 iterations of 2-byte blocks, 
                     // but the ASM does inx2 (x+=2). 
                     // Loop terminates when x == 20 (0x14).
        
        // Copy pair
        ram[0x0E1D + x] = ram[0x0AAF + y]; // lda $0aaf,y / sta $0e1d,x
        ram[0x0E1E + x] = ram[0x0AB0 + y]; // lda $0ab0,y / sta $0e1e,x
        
        x += 2; // inx2
        if (x == 20) break; // cpx #14 (0x14 = 20) / beq @d341
        
        y += 2; // iny2
        if (y != (offset + 20)) { // cpy #14 is checking the relative offset from 
                                  // the start of the inner loop's Y growth.
                                  // Wait: cpy #14 is an absolute check. 
                                  // If Y reaches 20, it resets to 0.
            continue; 
        }
        
        // This part of the ASM loop is slightly unusual:
        // if (y == 20) { y = 0; jmp @d321; }
        // This suggests the palette source is a wrapping buffer of 20 bytes.
        if (y == 20) {
            y = 0;
        }
    }
}

// PITFALLS: 7 (asl A in 8-bit mode truncated to uint8_t)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x7A=1, 0x79=1, 0x0AAF=1, 0x0AB0=1
//   output_ram:  0x0E1D=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::ShiftWhirlpoolPal ($D3:0A)