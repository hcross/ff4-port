// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   1. Clears ram[$DE] (likely a flag or scratchpad).
//   2. Reads an AI condition index from ram[$289C].
//   3. Uses this index (multiplied by 2 for word alignment) to look up
//      a target address in AICondTbl.
//   4. Stores the target address in ram[$80-$81] and the return address
//      (or a specific marker) $0300 in ram[$82-$83].
//   5. Performs an indirect jump to the retrieved address.
static void CheckAICond_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xDE] = 0;                                 // stz $de

    uint8_t index = ram[0x289C];                   // lda $289c
    uint8_t shifted_index = (uint8_t)(index << 1); // asl (Pitfall 7: truncated to 8-bit)
    
    // X is inherited as 16-bit (xf=0). tax moves the 8-bit A into X.
    // AICondTbl is a table of 16-bit addresses.
    // We simulate the table lookup: lda AICondTbl, x / sta $80 ...
    uint16_t target_addr = read16(&snes->rom[0xAICondTbl], shifted_index); 
    
    // Note: In the actual emulator/harness, AICondTbl is a constant offset.
    // Since the asm uses `lda f:AICondTbl, x`, we write the result to ram.
    ram[0x80] = target_addr & 0xFF;               // sta $80
    ram[0x81] = (target_addr >> 8) & 0xFF;          // sta $81
    
    ram[0x82] = 0x03;                              // lda #$03 / sta $82
    // Note: $83 is implicitly 0 from previous state or not touched here, 
    // but the asm only sets $82.

    // jml [$0080] is an indirect jump. In the snesrev pattern, 
    // we delegate the jump target to the emulator to handle the 
    // actual function execution and return.
    run_emulated_func(snes, target_addr);
}

// PITFALLS: 7 (ASL truncation to 8-bit), 8 (Inherited mf=true, xf=false)
// HELPERS: read16, run_emulated_func
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x289C=1
//   output_ram:  0xDE=1, 0x80=1, 0x81=1, 0x82=1
//   entry_mode:  mf=true, xf=false, dp=0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::CheckAICond ($BC:CE)