// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Computes AI script pointer: ptr = base + (script_id * 60)
// Stores result at $360C indexed by ($361C * 2).
static void GetAIScriptPtr_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Setup Mult8: $DF = script_id, $E1 = 60
    ram[0xDF] = ram[0x35BE];
    ram[0xE1] = 0x3C;
    mult8_emu(snes);  // result → $E3-$E4
    
    // Index = $361C * 2 (via asl)
    uint8_t idx = ram[0x361C];
    uint8_t shifted = (uint8_t)(idx << 1);  // asl in 8-bit (Pitfall 7)
    uint16_t x = shifted;  // tax: zero-extend to 16-bit
    
    // Ptr = base + offset
    uint16_t base = read16(ram, 0x35BF);
    uint16_t offset = read16(ram, 0xE3);
    uint16_t ptr = base + offset;
    write16(ram, 0x360C + x, ptr);
}

// PITFALLS: 1 (DB=$7E required), 7 (ASL truncation in 8-bit mode)
// HELPERS: mult8_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x35BE=1, 0x361C=1, 0x35BF=2
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: auto
// CUSTOM_SPIKE: yes
REVERSED_FUNCTION: battle::GetAIScriptPtr ($B7:4C)