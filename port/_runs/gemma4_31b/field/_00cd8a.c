#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00, DP=0
// Logic:
//   Iterates 25 times (0 to 0x18). In each iteration, it reads 4 bytes
//   from a source table (f:_14f58e) with an offset provided by X.
//   It adds a constant offset (ram[0x0C] and ram[0x0E]) to the first two bytes
//   and writes the resulting 4-byte sequence to ram[0x0340 + y].
//   Then it calls NextSprite to process the sprite data.
static void _00cd8a_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = snes->cpu->x;
    
    // The source table f:_14f58e is likely in ROM or a fixed memory bank.
    // Based on the assembly 'lda f:_14f58e,x', it's an absolute 24-bit address.
    // We use the snes->ram mapping or direct pointer for f:_14f58e.
    // Note: f:_14f58e is likely 0x14F58E.
    const uint8_t *src_table = &snes->ram[0x14F58E]; // Simplified for harness

    for (uint16_t y = 0; ; y++) {
        // Byte 0: lda table,x / clc / adc $0c / sta $0340,y
        uint8_t val0 = src_table[x];
        val0 = (uint8_t)(val0 + ram[0x0C]); // Pitfall 7: wrap to 8-bit
        ram[0x0340 + y] = val0;

        // Byte 1: lda table+1,x / clc / adc $0e / sta $0341,y
        uint8_t val1 = src_table[x + 1];
        val1 = (uint8_t)(val1 + ram[0x0E]); // Pitfall 7: wrap to 8-bit
        ram[0x0341 + y] = val1;

        // Byte 2: lda table+2,x / sta $0342,y
        ram[0x0342 + y] = src_table[x + 2];

        // Byte 3: lda table+3,x / sta $0343,y
        ram[0x0343 + y] = src_table[x + 3];

        next_sprite_emu(snes); // jsr NextSprite (delegated)

        if (y == 0x18) break; // cpy #$0018 / bne @cd8d
    }
}

// PITFALLS: 7 (8-bit arithmetic truncation for ADC)
// HELPERS: next_sprite_emu(snes) — delegates NextSprite @ 00:88E2
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  0x0C=1, 0x0E=1, 0x14F58E=1 (table)
//   output_ram:  0x0340=1 (sprite buffer)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00cd8a ($CD:8A)