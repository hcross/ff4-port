#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$FE, DP=0
// This routine decodes a run-length encoded (RLE) tilemap from the ROM
// into a destination buffer in WRAM.
//
// Logic:
// 1. Load a pointer from UndergroundTilemapPtrs using index at ram[$3D].
// 2. Read bytes from UndergroundTilemap.
// 3. If byte is positive (bpl): write single byte to destination and advance.
// 4. If byte is negative (bmi): treat as RLE. The absolute value (byte & 0x7F)
//    is the count, and the following byte is the tile value to repeat.
static void DecodeUndergroundTilemap_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Ldx $3d / lda ptrs, x / sta $3d / lda ptrs+1, x / sta $3e
    uint8_t index = ram[0x3D];
    uint16_t map_ptr = (uint16_t)(snes->rom[0x0000 + (index * 2)] | (snes->rom[0x0000 + (index * 2) + 1] << 8));
    // Note: The asm stores this ptr back into ram[0x3D] and ram[0x3E]
    ram[0x3D] = map_ptr & 0xFF;
    ram[0x3E] = (map_ptr >> 8) & 0xFF;

    uint16_t current_ptr = map_ptr;
    uint16_t dest_idx = read16(ram, 0x40);

    while (1) {
        // lda UndergroundTilemap, x (where x is the ptr from $3D/$3E)
        uint8_t val = snes->rom[current_ptr];

        if (val & 0x80) { // bpl @fe38 not taken (Negative/RLE)
            uint8_t count = val & 0x7F; // and #$7f
            current_ptr++; // move to the value byte
            uint8_t tile = snes->rom[current_ptr];
            current_ptr++;

            // RLE Fill loop (@fe22)
            for (uint8_t i = 0; i < count; i++) {
                ram[0x7F5C71 + dest_idx] = tile; // sta $7f5c71, x
                dest_idx++;
            }
        } else { // bpl @fe38 taken (Single byte)
            ram[0x7F5C71 + dest_idx] = val; // sta $7f5c71, x
            dest_idx++;
            current_ptr++;
        }

        // stx $40 / txa / beq @fe4c
        write16(ram, 0x40, dest_idx);
        if (dest_idx == 0) break; // Wrap-around check (beq @fe4c)

        // The asm does a weird check: if dest_idx != 0, it increments current_ptr
        // and jumps back to @fe10. However, since it's a tilemap,
        // the loop usually terminates when a specific end-marker or 
        // boundary is hit, but here it's governed by the 'beq @fe4c'.
        // In the asm, the loop continues unless the index wraps to 0.
        // But since it's a tilemap decode, there is usually a limit.
        // Looking at the asm, if it didn't wrap, it just loops.
        // For parity, we must ensure we don't loop infinitely if the ROM 
        // doesn't provide a termination. However, the original ASM 
        // logic effectively loops until dest_idx wraps or another 
        // external condition is met. 
        // Actually, the 'beq @fe4c' is likely a sentinel check or
        // a check for a specific value loaded into A. 
        // In this specific routine, the txa / beq refers to the dest_idx.
        
        // Correction: In the ASM, the check 'txa / beq @fe4c' happens 
        // after incrementing the destination pointer. 
        // This is a common pattern for "process until wrap" or 
        // "process until a certain size".
        
        // If the loop is intended to be finite, the ROM data must 
        // lead to a state where the loop breaks.
        if (dest_idx == 0) break; 
        
        // To avoid infinite loop in C simulation if the original
        // had a different termination, we'd need the exact ROM boundary.
        // But following ASM:
        // ldx $3d / inx / stx $3d (for single) or inx2 / stx $3d (for RLE)
        // This is already handled by current_ptr increments.
    }
}

// PITFALLS: 6 (Mode A 8-bit vs 16-bit), 8 (Inherited mf=true). 
// The routine uses 8-bit A for tile values/counts and 16-bit X for 
// the destination offset.
//
// HELPERS: none
//
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3D=1, 0x40=2
//   output_ram:  0x7F5C71=1 (buffer fill), 0x40=2 (updated index)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFE
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::DecodeUndergroundTilemap ($FE:00)