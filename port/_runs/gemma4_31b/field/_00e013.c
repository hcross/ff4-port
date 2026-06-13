#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 8-bit (xf=1), Y 8-bit (xf=1), DP=0x0, DB=0xE0
// This routine updates sprite data in WRAM (0x0350+) based on coordinates and a lookup table.
//
// Logic:
// 1. Validates entry conditions based on ram[$AD] and ram[$7A].
// 2. Calculates an index 'y' = (ram[$AD] - 0x10) & 0xFC.
// 3. Writes a series of values to RAM offsets starting at $0350 + y.
// 4. Calls set_sprite_msb_emu if certain coordinate bits are set.
static void _00e013_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Check conditions for skipping the logic
    if (ram[0xAD] != 0x20) {
        uint8_t val7a = ram[0x7A];
        uint8_t shifted = (uint8_t)(val7a >> 1); // lsr / bcc
        if (shifted != 0) {
            return;
        }
    }

    // Calculate index y: (ram[$AD] - 0x10) & 0xFC
    uint8_t y = (uint8_t)((ram[0xAD] - 0x10) & 0xFC);

    // First sprite block
    ram[0x0350 + y] = ram[0x0C];
    if (ram[0x0D] & 0x01) { // beq @e038
        set_sprite_msb_emu(snes, 0x14);
    }
    ram[0x0351 + y] = ram[0x0E];

    // Table lookup for first block (f:_15b8c9)
    // Assuming f:_15b8c9 is a label in ROM/RAM. 
    // Based on the offset, this is usually processed as: ram[base + x]
    // Since X is not modified in this routine, it is an inherited input.
    uint16_t x = snes->cpu->x;
    uint8_t *table = &snes->ram[0x158C9]; // Adjusted to ram index per project layout
    ram[0x0352 + y] = table[x];
    ram[0x0353 + y] = table[x + 1];

    // Second sprite block coordinates
    ram[0x0354 + y] = (uint8_t)(ram[0x0C] + 0x08); // clc / adc #$08
    
    // Check for MSB update on second block
    uint8_t carry_val = (uint16_t)ram[0x0C] + 0x08 > 0xFF;
    uint8_t low_byte = (uint8_t)(ram[0x0D] + 0x00 + carry_val); // adc #$00
    if ((low_byte & 0x01) != 0) { // beq @e061
        set_sprite_msb_emu(snes, 0x15);
    }

    ram[0x0355 + y] = ram[0x0E];
    ram[0x0356 + y] = table[x + 2];
    ram[0x0357 + y] = table[x + 3];
}

// PITFALLS: 7 (8-bit arithmetic truncation for ADC/SBC), 8 (Assumed mf=true/xf=true based on 8-bit absolute loads/stores and index usage)
// HELPERS: set_sprite_msb_emu(snes, value)
// CONTRACT:
//   inputs_reg:  a=none, x=8, y=none
//   inputs_ram:  0xAD=1, 0x7A=1, 0x0C=1, 0x0D=1, 0x0E=1, 0x158C9=4
//   output_ram:  0x0350=8 (writes span 0x0350+y to 0x0357+y)
//   entry_mode:  mf=true, xf=true, dp=0x0, db=0xE0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00e013 ($E0:0013)