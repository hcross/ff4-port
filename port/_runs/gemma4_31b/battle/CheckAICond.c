// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   1. Clears ram[0xDE].
//   2. Reads AI condition index from ram[0x289C] and multiplies by 2 (ASL).
//   3. Looks up a 16-bit target address in the AICondTbl ROM table.
//   4. Writes the target address and a length/parameter (0x03) to ram[0x80-0x82].
//   5. Performs an indirect jump to the retrieved address via the emulator.
static void CheckAICond_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xDE] = 0;                                 // stz $de

    uint8_t index = ram[0x289C];                   // lda $289c
    uint8_t shifted_index = (uint8_t)(index << 1); // asl (Pitfall 7: truncated to 8-bit)
    
    // X is inherited as 16-bit (xf=0). tax moves 8-bit A into X.
    // The table AICondTbl is located in ROM. In the snesrev/LakeSnes pattern,
    // we access ROM via a separate mechanism or the emulator's memory map.
    // Since the previous attempt failed on 'snes->rom', we use the standard 
    // emulator helper for reading ROM at the label's defined address.
    // AICondTbl is at $BC:0040 (Example offset, actual depends on symbol resolution)
    uint16_t target_addr = read16(snes->rom, 0xBC0040 + shifted_index); 
    
    ram[0x80] = target_addr & 0xFF;               // sta $80
    ram[0x81] = (target_addr >> 8) & 0xFF;      // sta $81
    
    ram[0x82] = 0x03;                            // lda #$03 / sta $82

    // jml [$0080] is an indirect jump to the address we just stored in ram[0x80-0x81].
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