// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Input: A register (8-bit) = element flags
// Output: A register (8-bit) = anti-element flags
// Logic:
//   1. Extract bits 1,4 (holy,ice) → shift to bits 0,3 (dark,fire)
//   2. Extract bits 0,3 (dark,fire) → shift to bits 1,4 (holy,ice)
//   3. OR both results to form anti-element mask
static void GetAntiElem_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    uint8_t a = cpu->a;              // input in A (8-bit)
    uint8_t temp = a & 0x12;         // and #$12
    temp >>= 1;                      // lsr (bit 1→0, bit 4→3)
    snes->ram[0xAA] = temp;          // sta $aa
    uint8_t a2 = snes->ram[0xA9];    // lda $a9
    uint8_t temp2 = a2 & 0x09;       // and #$09
    temp2 <<= 1;                     // asl (bit 0→1, bit 3→4)
    uint8_t result = temp2 | temp;   // ora $aa
    cpu->a = result;                 // result in A
    // Update flags to match 8-bit mode RTS behavior
    cpu->z = (result == 0);
    cpu->n = (result & 0x80) != 0;
}

// PITFALLS: 1 (DB=$7E assumed for $aa/$a9 access),
//           6 (A is 8-bit: mf=1),
//           7 (upper byte of A preserved in 8-bit mode)
// HELPERS: none (fully translated)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0xA9=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetAntiElem ($9D:9E)