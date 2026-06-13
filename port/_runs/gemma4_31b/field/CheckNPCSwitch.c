#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$C3, DP=0
// Logic:
//   Checks if a specific NPC switch is active.
//   1. Extracts a 3-bit index from A (bits 0-2) and a 5-bit base offset (bits 3-7).
//   2. Adjusts the base offset by adding 0x20 if ram[0x0FE5] has bit 7 set (bmi) 
//      OR if ram[0x1701] is non-zero (beq skip).
//   3. Loads a bitmask from ROM bank F at $12E0 + offset.
//   4. Iterates through the mask, shifting right and decrementing the index.
//   5. Returns 1 in A if the bit corresponding to the index was set, 0 otherwise.
static void CheckNPCSwitch_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = (uint8_t)snes->cpu->a;

    ram[0x07] = a & 0x07;              // and #$07 / sta $07
    
    uint8_t offset = (uint8_t)(a >> 3); // lsr3 (3 shifts right)
    ram[0x3D] = offset;                // sta $3d

    // Handle offset adjustment based on flags/RAM
    bool adjust = false;
    if (ram[0x0FE5] & 0x80) {          // lda $0fe5 / bmi @c405
        adjust = true;
    } else if (ram[0x1701] != 0) {     // lda $1701 / beq @c40c
        adjust = true;
    }

    if (adjust) {
        ram[0x3D] = (uint8_t)(ram[0x3D] + 0x20); // clc / adc #$20 / sta $3d
    }

    ram[0x3E] = 0;                     // stz $3e
    uint8_t y = ram[0x07];             // lda $07 / tay
    uint8_t x = ram[0x3D];             // ldx $3d

    // ROM access: Bank F, $12E0. UseLakeSnes rom access via read_rom_byte (or equivalent mapping)
    // In this harness, we assume a helper or direct access to the mapped ROM space.
    // Since snes->rom failed, we use the standard harness rom access pattern.
    uint8_t mask = snes->rom_data[0x12E0 + x]; // lda f:$0012e0,x

    // Loop: Check bitmask
    while (1) {
        if (mask == 0) break;          // cpy #0 / beq @c421
        
        // The ASM performs lsr (A) and dey. The loop terminates when y=0 or mask=0.
        // The final result is (mask & 1) ONLY IF we exited via y=0 or the loop.
        // But we need to track the carry for the final result.
        if (y == 0) break;
        
        mask = (uint8_t)(mask >> 1);   // lsr (Pitfall 7)
        y--;                          // dey
    }

    // @c421: result = (mask & 1)
    // lsr (shifts bit 0 into C), lda #0, adc #0 (A = 0 + 0 + C)
    uint8_t result = (mask & 1);
    snes->cpu->a = result;
}

// PITFALLS: 6 (Mode A 8-bit), 7 (Shift truncation using uint8_t)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x0FE5=1, 0x1701=1
//   output_ram:  0x07=1, 0x3D=1, 0x3E=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC3
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::CheckNPCSwitch ($C3:EF)