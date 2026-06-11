// Modify a battle variable based on operation type encoded in RAM[0xA9]
// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// Input A: variable selector (index + 0xF4, e.g., 0xF4 = var 0)
// RAM[0xA9]: operation descriptor
//   bits 7-6: 00=add, 01=subtract, 1x=set
//   bits 5-0: operand value
// Modifies: RAM[0x35F3 + (A-0xF4)] and RAM[0xA9]
static void ChangeBattleVar_c(Snes *snes, uint8_t var_code) {
    uint8_t *ram = snes->ram;
    
    // sec / sbc #$f4 / tay - compute variable index
    uint8_t var_idx = (uint8_t)(var_code - 0xF4);  // Pitfall 7
    
    // lda $a9 / pha / and #$3f / sta $a9 / pla
    uint8_t op_byte = ram[0xA9];
    uint8_t operand = op_byte & 0x3F;
    ram[0xA9] = operand;
    
    // and #$c0 - extract operation type
    uint8_t op_type = op_byte & 0xC0;
    
    uint8_t result;
    if (op_type == 0) {
        // add: clc / lda $a9 / adc $35f3,y
        result = (uint8_t)(operand + ram[0x35F3 + var_idx]);  // Pitfall 7
    } else if ((op_type & 0x80) == 0) {
        // subtract: sec / lda $35f3,y / sbc $a9
        result = (uint8_t)(ram[0x35F3 + var_idx] - operand);  // Pitfall 7
    } else {
        // set: lda $a9
        result = operand;
    }
    
    // sta $35f3,y
    ram[0x35F3 + var_idx] = result;
}

// PITFALLS: 1 (DB=$7E required for absolute addressing), 7 (8-bit arithmetic
// truncation on sbc, adc, and intermediate results), 8 (mode A 8-bit inherited
// from battle module convention)
// HELPERS: (none)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0xA9=1
//   output_ram:  none (dynamic output at 0x35F3+(a-0xF4), side effect at 0xA9)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes
// REVERSED_FUNCTION: battle::ChangeBattleVar ($03:B877)