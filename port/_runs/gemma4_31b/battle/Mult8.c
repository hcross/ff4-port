// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// 
// This routine performs 8-bit multiplication (A * ram[0xDF]).
// It uses a shift-and-add algorithm over 16 iterations.
// The multiplier is shifted out of ram[0xE1] (initialized by the caller),
// and the product is accumulated and shifted into ram[0xE3] and ram[0x394D].
//
// Note: The ASM does 'ror $e1' as the first loop instruction.
// This implies the value of A was stored in ram[0xE1] by the caller
// before the call, or the routine relies on that specific memory location.
static void Mult8_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // Setup
    ram[0xE0] = 0;
    ram[0xE2] = 0;
    ram[0xE3] = 0;
    ram[0x394D] = 0;

    // longa: A is now 16-bit
    cpu->mf = false;
    
    uint16_t x = 0x0010; // ldx #$0010

    for (int i = 0; i < 16; i++) {
        // ror $e1
        // Shift ram[0xE1] right, pulling from Carry, pushing bit 0 to Carry
        uint8_t e1 = ram[0xE1];
        uint8_t e1_bit0 = e1 & 1;
        ram[0xE1] = (uint8_t)((e1 >> 1) | (cpu->c << 7));
        cpu->c = (e1_bit0 != 0);

        if (cpu->c) { // bcc @83fb -> Not taken if Carry=1
            cpu->c = false; // clc
            uint8_t multiplicand = ram[0xDF];
            uint8_t current_sum = ram[0x394D];
            
            // Pitfall 7: 8-bit addition truncation
            uint8_t res = (uint8_t)(multiplicand + current_sum);
            ram[0x394D] = res;
            
            // Update carry for the addition (for the subsequent ror)
            cpu->c = ((uint16_t)multiplicand + (uint16_t)current_sum) > 0xFF;
        }

        // ror $394d
        uint8_t r394d = ram[0x394D];
        uint8_t r394d_bit0 = r394d & 1;
        ram[0x394D] = (uint8_t)((r394d >> 1) | (cpu->c << 7));
        cpu->c = (r394d_bit0 != 0);

        // ror $e3
        uint8_t re3 = ram[0xE3];
        uint8_t re3_bit0 = re3 & 1;
        ram[0xE3] = (uint8_t)((re3 >> 1) | (cpu->c << 7));
        cpu->c = (re3_bit0 != 0);

        x--; // dex
        if (x == 0) break; // bne @83ee
    }

    // shorta0: A = D (usually 0 in battle), A mode = 8-bit
    cpu->a = cpu->dp; 
    cpu->mf = true;
}

// PITFALLS: 7 (8-bit ADC and ROR truncation/carry handling)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xDF=1, 0xE1=1
//   output_ram:  0xE3=1, 0x394D=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::Mult8 ($83:E0)