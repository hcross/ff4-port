// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine calculates the opposite/anti-elemental bits.
// It takes an element mask in A, transforms it, and merges it with 
// bits from $a9 to produce a final mask stored in A and mirrored in $aa.
// Logic:
// 1. Mask A with $12 (Holy/Ice), shift right (becoming Darkness/Fire), store in $aa.
// 2. Mask $a9 with $09 (Darkness/Fire), shift left (becoming Holy/Ice).
// 3. Combine (OR) these two masks.
static uint16_t GetAntiElem_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = (uint8_t)snes->cpu->a;

    a &= 0x12;                       // and #$12
    uint8_t shifted_a = (uint8_t)(a >> 1); // lsr A (Pitfall 7)
    ram[0xAA] = shifted_a;           // sta $aa

    uint8_t a9 = ram[0xA9];          // lda $a9
    a9 &= 0x09;                      // and #$09
    uint8_t shifted_a9 = (uint8_t)(a9 << 1); // asl A (Pitfall 7)
    
    uint8_t result = shifted_a9 | shifted_a; // ora $aa
    
    return (uint16_t)result;         // rts (result in A)
}

// PITFALLS: 7 (8-bit shifts truncated to uint8_t to avoid C promotion)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0xA9=1
//   output_ram:  0xAA=1
//   entry_mode:  mf=true, xf=false, dp=0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetAntiElem ($9D:9E)