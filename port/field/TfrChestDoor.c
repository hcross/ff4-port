#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$C7 (ROM), DP=0
// This routine transfers chest door coordinates to the VRAM DMA registers ($2115-2118).
// It handles a coordinate offset (+0x20) and updates the source RAM.
// 
// Note: Memory accesses to $2115-2118 are hardware registers (VRAM DMA).
// In this reimplementation, we map these to snes->ram if the harness 
// treats them as memory-mapped I/O, or use direct pointer offsets.
static void TfrChestDoor_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $d4 / bne @c7ea
    if (ram[0xD4] == 0) {
        return;
    }

    // @c7ea: stz $d4
    ram[0xD4] = 0;

    // lda #$80 / sta $2115
    ram[0x2115] = 0x80;

    // ldx $06fe / stx $2116
    uint16_t x = read16(ram, 0x06FE);
    write16(ram, 0x2116, x);

    // ldx $0700 / stx $2118 (Note: 65816 absolute write to $2118)
    x = read16(ram, 0x0700);
    write16(ram, 0x2118, x);

    // ldx $0702 / stx $2118 (Overwrites previous $2118)
    x = read16(ram, 0x0702);
    write16(ram, 0x2118, x);

    // lda $06fe / clc / adc #$20 / sta $06fe
    uint8_t a = ram[0x06FE];
    a = (uint8_t)(a + 0x20); // Pitfall 7: Truncate to 8-bit
    ram[0x06FE] = a;

    // lda $06ff / adc #$00 / sta $06ff
    // Note: Carry from the previous ADC is preserved here
    uint8_t carry = (a < ram[0x06FE]); // Simplified carry check for C
    // In ASM, the carry is determined by the result of (ram[0x06FE] + 0x20)
    // Since it's a simple 8-bit addition, we need to track it carefully.
    
    // Re-calculating exactly as 65816 would:
    uint16_t sum = (uint16_t)ram[0x06FE] + 0x20; // This was actually the previous step
    // The previous 'sta $06fe' happens after 'adc #$20'.
    // Let's trace again:
    // [A = ram[0x06FE]] -> [A = A + 0x20] -> [ram[0x06FE] = A]
    // Then [A = ram[0x06FF]] -> [A = A + 0 + Carry] -> [ram[0x06FF] = A]
    
    // Correct carry propagation:
    uint8_t val_fe = ram[0x06FE]; // Current value before modification in logic
    // Wait, the ASM modifies $06fe first.
    // Re-evaluating sequence:
    uint8_t old_fe = ram[0x06FE];
    uint8_t new_fe = (uint8_t)(old_fe + 0x20);
    bool carry_out = (new_fe < old_fe); 
    ram[0x06FE] = new_fe;

    uint8_t val_ff = ram[0x06FF];
    val_ff = (uint8_t)(val_ff + 0 + (carry_out ? 1 : 0));
    ram[0x06FF] = val_ff;

    // ldx $06fe / stx $2116
    x = read16(ram, 0x06FE);
    write16(ram, 0x2116, x);

    // ldx $0704 / stx $2118
    x = read16(ram, 0x0704);
    write16(ram, 0x2118, x);

    // ldx $0706 / stx $2118 (Overwrites previous $2118)
    x = read16(ram, 0x0706);
    write16(ram, 0x2118, x);
}

// PITFALLS: 7 (8-bit arithmetic truncation and carry propagation)
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00D4=1, 0x06FE=2, 0x0700=2, 0x0702=2, 0x0704=2, 0x0706=2
//   output_ram:  0x2118=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC7
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::TfrChestDoor ($C7:E5)