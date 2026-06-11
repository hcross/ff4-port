// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// No conditional branches at entry, so no Z/N flag contract.
//
// Logic:
//   1. Multiply monster_script_id (ram[$35be]) by 0x3C (60 decimal)
//   2. Calculate array index = 2 * ram[$361c] (doubled for 16-bit word addressing)
//   3. Add multiplication result to base pointer at $35bf:$35c0 (16-bit)
//   4. Store computed pointer at $360c+index (16-bit LE)
static void GetAIScriptPtr_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Setup Mult8 operands: ram[$35be] * 0x3C
    // Result will be in $e3:$e4 (16-bit LE)
    ram[0xDF] = ram[0x35BE];  // lda $35be / sta $df
    ram[0xE1] = 0x3C;         // lda #$3c / sta $e1
    mult8_emu(snes);          // jsr Mult8 (delegated)
    
    // Calculate index: 2 * ram[$361c]
    // ASM: lda $361c / asl / tax
    // asl in 8-bit mode truncates to 8 bits (Pitfall 7), then tax extends to 16-bit X
    uint16_t x = (uint16_t)((uint8_t)(ram[0x361C] << 1));
    
    // 16-bit addition: base_ptr + mult_result
    // ASM: clc / lda $35bf / adc $e3 / sta $360c,x
    //           lda $35c0 / adc $e4 / sta $360d,x
    uint16_t base_ptr = read16(ram, 0x35BF);
    uint16_t mult_result = read16(ram, 0xE3);
    uint16_t ai_script_ptr = base_ptr + mult_result;
    write16(ram, 0x360C + x, ai_script_ptr);
}

// PITFALLS: 7 (asl in 8-bit mode: (ram[0x361C] << 1) truncated to uint8_t before extension to uint16_t)
// HELPERS: mult8_emu(snes) — delegates Mult8 @ $B7:83E0
//          read16/write16 — little-endian 16-bit accessors over ram[]
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x35BE=1, 0x361C=1, 0x35BF=2
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: auto
//   CUSTOM_SPIKE: yes
//
// Note: output address is dynamically calculated (0x360C + 2*ram[0x361C]),
// so automatic spike generation cannot determine a fixed observable address.
// Manual spike harness required.
REVERSED_FUNCTION: battle::GetAIScriptPtr ($B7:4C)