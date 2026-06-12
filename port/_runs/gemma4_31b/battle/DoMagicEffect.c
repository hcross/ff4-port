// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   1. Double the input value in A (index * 2).
//   2. Use that value as an index into MagicEffectTbl to fetch a 16-bit address.
//   3. Store that address in ram[0x80-0x81] and a constant 0x03 in ram[0x82].
//   4. Perform a jump (jml) to the address stored at 0x80.
//
// Since this is a jump table dispatcher, it effectively transfers control
// to a specific effect handler. In the C reimplementation, we must 
// emulate the jump by delegating the target function.
static void DoMagicEffect_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // asl A (8-bit mode)
    uint8_t index = (uint8_t)(snes->cpu->a << 1); // Pitfall 7: truncate to 8-bit
    
    // tax / lda f:MagicEffectTbl,x / sta $80 / lda f:MagicEffectTbl+1,x / sta $81
    // The table is located in the ROM. We resolve the address based on index.
    // Note: f:MagicEffectTbl is a symbol. For the emulator to work, 
    // we simulate the memory writes to the "jump stack" at $80-$82.
    uint16_t target_pc = read16(&snes->rom[MAGIC_EFFECT_TBL + index], 0); 
    
    write16(ram, 0x80, target_pc);
    ram[0x82] = 0x03; // sta $82
    
    // jml [$0080]
    // This is an indirect jump. We delegate the execution to the 
    // target address fetched from the table.
    run_emulated_func(snes, target_pc);
}

// PITFALLS: 7 (asl in 8-bit mode truncated to uint8_t)
// HELPERS: run_emulated_func(snes, pc) — implements the jml [$0080]
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x80=2, 0x82=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::DoMagicEffect ($D2:97)