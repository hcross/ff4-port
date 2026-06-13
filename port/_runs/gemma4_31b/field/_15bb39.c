#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$BB, DP=0
// Logic:
//   Checks conditions based on RAM values $c8, $7a, and $b9.
//   If conditions are met, it calculates an index from $b9,
//   then copies 32 bytes (0x20) from a ROM table (_15bb6a) 
//   starting at the calculated offset into WRAM $04C0.
static void field_15bb39_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Condition 1: lda $c8 / bne @bb68
    if (ram[0xC8] != 0) return;

    // Condition 2: lda $7a / and #$01 / bne @bb68
    if ((ram[0x7A] & 0x01) != 0) return;

    // Calculation of index from $b9
    uint8_t val = ram[0xB9];
    uint8_t result = val;
    
    // sec / sbc #$10 / cmp #$10 / bne @bb52 / dec
    // This block simulates: if (val - 0x10 == 0x10) result = val - 1; else result = val;
    // In 8-bit: (val - 0x10) == 0x10 is equivalent to val == 0x20
    if (val == 0x20) {
        result = val - 1;
    }

    // @bb52: and #$0c / asl3
    // result = (result & 0x0C) << 3
    uint8_t index = (uint8_t)((result & 0x0C) << 3); // Pitfall 7: wrap cast

    // Loop: copy 32 bytes from ROM table _15bb6a + index to ram[0x04C0]
    // Note: _15bb6a is a constant address in ROM.
    // Since we are translating to C, we access the ROM data via the snes instance.
    // Assuming snes->rom is available or the harness provides a read_rom helper.
    // For the purpose of this translation, we use the address provided by the symbol.
    uint32_t rom_base = 0x15BB6A; 
    for (uint8_t i = 0; i < 0x20; i++) {
        // lda _15bb6a,x / sta $04c0,y
        ram[0x04C0 + i] = snes->rom[rom_base + index + i];
    }
}

// PITFALLS: 7 (Arithmetic truncation: (result & 0x0C) << 3 must be uint8_t)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00C8=1, 0x007A=1, 0x00B9=1
//   output_ram:  0x04C0=32
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xBB
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_15bb39 ($BB:39)