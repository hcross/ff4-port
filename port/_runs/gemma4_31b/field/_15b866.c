#include "snes/snes.h"

// Logic:
// Calculates a target value ($06) based on input A and state flags ($7a, $a1, $c8).
// If conditions are met, it writes a sequence of 8 bytes to WRAM $04C0-$04C7.
// This sequence includes a constant offset (0x70/0x78), the calculated value,
// and 4 bytes from a table (ROM) indexed by the processed input A.
static void _15b866_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = (uint8_t)snes->cpu->a;

    // cmp #$10 / bcc @b86c
    if (a >= 0x10) {
        a = 0x10; // lda #$10
    }

    a &= 0xFC;    // and #$fc
    uint8_t x_idx = a; // tax (A 8-bit, X 16-bit but value is 0-16)

    // cmp #$10 / beq @b879
    if (x_idx != 0x10) {
        // lda $7a / and #$01 / bne @b8c8
        if ((ram[0x7A] & 0x01) != 0) {
            return;
        }
    }

    // @b879: lda $c8 / bne @b8c8
    if (ram[0xC8] != 0) {
        return;
    }

    // Logic for $06 based on $a1 flags
    uint8_t val_06;
    if ((ram[0xA1] & 0x08) != 0) {
        val_06 = 0xFE; // lda #$fe / jmp @b890
    } else if ((ram[0xA1] & 0x04) != 0) {
        val_06 = 0x00; // lda #$00 / @b88e
    } else {
        val_06 = 0xFE; // fallthrough to @b889
    }
    ram[0x06] = val_06; // sta $06

    // Write to $04C0-$04C7
    ram[0x04C0] = 0x70;
    ram[0x04C1] = (uint8_t)(0x78 + ram[0x06]); // Pitfall 7: 8-bit adc truncation

    // Table lookup: f:_15b8c9
    // Using direct ROM access via snes->rom (assumed map for f:_15b8c9)
    // The exact pointer for f:_15b8c9 is 0x15B8C9 (Bank B8, offset 0x8C9)
    uint8_t *table = &snes->rom[0xB8 * 0x10000 + 0x8C9]; // Note: adjusted for ROM banking
    // Correction: Based on the provided ASM label f:_15b8c9, we use the calculated offset
    // In the actual harness, the table is accessed via a provided ROM pointer.
    // For parity, we treat the table as a known array.
    extern const uint8_t _15b8c9_table[]; 
    
    ram[0x04C2] = _15b8c9_table[x_idx];
    ram[0x04C3] = _15b8c9_table[x_idx + 1];
    
    ram[0x04C4] = 0x78;
    ram[0x04C5] = (uint8_t)(0x78 + ram[0x06]); // Pitfall 7
    ram[0x04C6] = _15b8c9_table[x_idx + 2];
    ram[0x04C7] = _15b8c9_table[x_idx + 3];
}

// PITFALLS: 7 (adc $06 truncated to 8-bit)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=16, y=none
//   inputs_ram:  0x06=1, 0x7A=1, 0xA1=1, 0xC8=1
//   output_ram:  0x04C0=1, 0x04C1=1, 0x04C2=1, 0x04C3=1, 0x04C4=1, 0x04C5=1, 0x04C6=1, 0x04C7=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_15b866 ($B8:66)