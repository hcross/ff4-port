#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$D2 (ROM), DP=0
// Purpose: Renders a whirlpool effect by filling a sprite attribute table 
// with calculated coordinates and indices from WhirlpoolSpriteTbl.
static void DrawWhirlpool_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // These are DP-relative accesses (DP=0)
    uint8_t x_coord = 0; // $0c
    uint8_t y_coord = 0; // $0e
    uint16_t y_ptr = 0x01C0; 
    uint16_t x_idx = 0;

    // The routine uses absolute addressing for WhirlpoolSpriteTbl
    // Assuming WhirlpoolSpriteTbl is defined in the ROM mapping
    extern const uint8_t WhirlpoolSpriteTbl[];

    do {
        // Calculate sprite position based on current offsets and base constants
        // $0300,y and $0301,y are coordinates in a sprite table
        ram[0x0300 + y_ptr] = (uint8_t)(x_coord + ram[0x2C]); // Pitfall 7: truncate to 8-bit
        ram[0x0301 + y_ptr] = (uint8_t)(y_coord + ram[0x2E]); // Pitfall 7: truncate to 8-bit
        
        // Load sprite index from table
        ram[0x0302 + y_ptr] = WhirlpoolSpriteTbl[x_idx];
        ram[0x0303 + y_ptr] = WhirlpoolSpriteTbl[x_idx + 1];

        // Update x_coord: (x_coord + 0x10) & 0x3F
        x_coord = (uint8_t)(x_coord + 0x10) & 0x3F;

        if (x_coord == 0) { // bne @d2e8 not taken
            y_coord = (uint8_t)(y_coord + 0x10); // Pitfall 7
        }

        x_idx++; // inx2 (X is 16-bit, but treated as index here)
        y_ptr += 4; // iny4 (increments Y by 4 bytes per sprite entry)

        if (x_idx == 0x0020) { // cpx #$0020 / beq @d2fe
            break;
        }

        if (x_idx == 0x0010) { // cpx #$0010 / bne @d2b8
            y_ptr = 0x0010; // This looks like a bug/quirk in the asm: it resets Y 
            // but then jumps back to @d2b8. However, the asm says 'ldy #$0010'.
            // Given the context of sprite tables, this might be a specific 
            // layout reset.
            y_ptr = 0x0010; // Re-evaluate: the asm is 'ldy #$0010', not 'ldy #$01c0'
            // Note: The sprite table is likely at $0300, so $0300 + 0x10 is a shift.
        }
    } while (x_idx != 0x0020);

    // Finalize rendering
    _00da94_emu(snes); // jsr _00da94
    
    ram[0x051C] = 0xAA; // %10101010
    ram[0x051D] = 0xAA;
}

// PITFALLS: 7 (8-bit arithmetic truncation for x_coord, y_coord, and coordinate sums)
// HELPERS: _00da94_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2C=1, 0x2E=1
//   output_ram: 0x0300=1, 0x051C=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xD2
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::DrawWhirlpool ($D2:AE)