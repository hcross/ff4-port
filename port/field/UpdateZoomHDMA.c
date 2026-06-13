#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$C1, DP=0
// Logic:
//   1. If ram[0x1700] == 3 (World Map), exit immediately.
//   2. Clear matrix elements hM7B/hM7C.
//   3. Setup Zoom HDMA data in RAM ($7F5A00 range).
//   4. Calculate offset based on ram[0xAD] (zoom level), 
//      diverging if ram[0x1704] == 0x06 (Big Whale event).
//   5. Trigger HDMA via registers $4340-$4357.
static void UpdateZoomHDMA_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Check if on world map
    if (ram[0x1700] == 3) { // lda $1700 / cmp #3 / bne @c16b (taken if != 3)
        return;
    }

    // The ASM has a bug or quirk: it branches to @c16b if A != 3.
    // If A == 3, it hits 'rtl'. If A != 3, it proceeds to HDMA setup.
    // Wait: the ASM says: bne @c16b ; return if not on world map.
    // This is a contradiction in the ASM comment. 
    // "bne @c16b" means "if A != 3, go to @c16b".
    // If A == 3, it falls through to 'rtl'.
    // Therefore: if (ram[0x1700] == 3) return;
    
    // Clear off-diagonal matrix elements (hM7B, hM7C are offsets/labels)
    // Assuming these are defined in the mapping, using placeholders for labels
    // based on the stz sequence.
    ram[0x211A] = 0; // hM7B lo
    ram[0x211B] = 0; // hM7B hi
    ram[0x211B] = 0; // hM7C lo (overlap/sequential)
    ram[0x211C] = 0; // hM7C hi
    // Note: Since labels aren't provided, I map them to typical HDMA matrix locations
    // but strictly follow the 'stz' count.

    ram[0x7F5A00] = 0xF0; // 224 scanlines
    ram[0x7F5A03] = 0xF0;

    if (ram[0x1704] == 0x06) { // Big Whale
        uint8_t val = ram[0xAD];
        val = (uint8_t)(val - 0x10); // sec / sbc #$10
        val &= 0xFE;                // and #$fe
        val = (uint8_t)(val + 0x22); // clc / adc #$22
        
        ram[0x7F5A02] = val;
        ram[0x7F5A05] = val;
    } else { // Not Big Whale
        uint8_t val = ram[0xAD];
        val = (uint8_t)(val - 0x10); // sec / sbc #$10
        val = (uint8_t)(val << 1);   // asl (Pitfall 7: truncate to 8-bit)
        val = (uint8_t)(val + 0x00); // clc / adc #$00
        
        ram[0x7F5A02] = val;
        ram[0x7F5A05] = val;
    }

    ram[0x7F5A01] = 0x00;
    ram[0x7F5A04] = 0xE0;
    ram[0x7F5A06] = 0x80; // HDMA terminator

    // HDMA Control Registers (Standard SNES HW registers mapped into RAM space for this project)
    // hHDMAEN is typically a bitmask in RAM or HW
    ram[0x2100] = 0; // Placeholder for hHDMAEN stz

    ram[0x4340] = 0x42;
    ram[0x4350] = 0x42;
    
    // hM7A low byte
    ram[0x4341] = 0x1B; // <hM7A ($211B)
    ram[0x4351] = 0x1E; // <hM7D ($211E)
    
    ram[0x4342] = 0x00; // low byte of $5A00
    ram[0x4343] = 0x5A; // high byte of $5A00 (stx $4342 implies 16-bit X)
    ram[0x4352] = 0x00;
    ram[0x4353] = 0x5A;

    ram[0x4344] = 0x7F;
    ram[0x4354] = 0x7F;
    ram[0x4347] = 0x7F;
    ram[0x4357] = 0x7F;
}

// PITFALLS: 7 (8-bit arithmetic truncation for asl/sbc/adc)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1700=1, 0x1704=1, 0xAD=1
//   output_ram:  0x7F5A00=1, 0x4340=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC1
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::UpdateZoomHDMA ($C1:63)