#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B4 (assuming field bank), DP=0
// Logic: 
//   1. Load value from $B2 (relative to DP=0).
//   2. Shift value left by 1 bit (effectively multiplying by 2).
//   3. Store result into $3D and $3E as a 16-bit little-endian value.
//   4. Return the result in X (16-bit).
static uint16_t GetDlgID_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    uint8_t val = ram[0xB2];
    ram[0x3E] = 0;               // stz $3e
    
    // asl A / rol $3e: 
    // Shift A left by 1. Bit 7 of A moves to Carry.
    // Then Rotate Left $3E through Carry.
    uint8_t shifted = (uint8_t)(val << 1); // Pitfall 7: truncate to 8-bit
    bool carry = (val & 0x80) != 0;
    
    ram[0x3D] = shifted;         // sta $3d
    ram[0x3E] = (uint8_t)((0 << 1) | carry); // rol $3e (since $3e was 0)
    
    return read16(ram, 0x3D);   // ldx $3d / rts
}

// PITFALLS: 7 (8-bit shift truncation), 1 (DB=$B4)
// HELPERS: read16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xB2=1
//   output_ram:  0x3D=1, 0x3E=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB4
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetDlgID ($B4:4D)