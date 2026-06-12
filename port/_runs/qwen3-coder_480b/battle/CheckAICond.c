// Entry mode: A 16-bit (mf=0), X 16-bit (xf=0), DB=0x7E, DP=0
// Input: $289C = AI condition index (0-255)
// Output: none (jump to condition handler via [$0080])
// This function computes a jump address from a table and performs an indirect jump.
static void CheckAICond_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0xDE] = 0;                    // stz $de
    uint16_t index = ram[0x289C];     // lda $289c
    index <<= 1;                      // asl (16-bit shift)
    uint16_t addr = read16(ram, 0x10000 + index); // lda f:AICondTbl,x (ROM at $10000)
    write16(ram, 0x80, addr);         // sta $80, sta $81
    ram[0x82] = 0x03;                 // lda #$03 / sta $82
    // Emulate the indirect jump `jml [$0080]` by calling the target directly
    // The target is in bank $03 (from ram[0x82]) and offset in $80-$81
    uint32_t target = (0x03 << 16) | addr;
    run_emulated_func(snes, target);
}

// PITFALLS: 1 (DB must be $7E for correct RAM access),
//           6 (A is 16-bit due to longa at call site),
//           9 (ROM access via f:AICondTbl,x is 16-bit at $10000 + index)
// HELPERS: run_emulated_func (to simulate jml [$0080])
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x289C=1
//   output_ram:  none
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::CheckAICond ($BC:CE)