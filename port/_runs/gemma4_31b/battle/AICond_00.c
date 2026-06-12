// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$BC, DP=0
// Purpose: Iterates through AI conditions in a table at $35D0. 
// Validates targets against a mask defined in WRAM and determines if 
// specific AI flags ($289D) trigger a a result in $DE.
static void AICond_00_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    GetAICondTarget_emu(snes);

    if (ram[0xDD] == 0) return; // beq @bd3c

    // Construct indirect pointer at $80/$81
    // clc / lda #$03 / adc $289e / sta $80
    uint16_t sum = 0x03 + ram[0x289E];
    ram[0x80] = (uint8_t)sum;
    
    // lda #$20 / adc #$00 / sta $81
    // Carry from previous ADC is propagated to $20
    uint8_t carry = (sum > 0xFF) ? 1 : 0;
    ram[0x81] = (uint8_t)(0x20 + carry);

    // clr_ax: A = X = D (DP=0)
    cpu->a = 0;
    cpu->x = 0;

    do {
        // lda $35d0,x / sta $a9
        // lda $35d1,x / sta $aa
        ram[0xA9] = ram[0x35D0 + cpu->x];
        ram[0xAA] = ram[0x35D1 + cpu->x];

        // ldy $a9 / cpy #$ffff
        // Y is 16-bit, loading 8-bit $a9 results in 0x00XX. 
        // 0x00XX can never equal 0xFFFF.
        uint16_t y = ram[0xA9]; 
        if (y == 0xFFFF) { // beq @bd28
            goto loop_continue;
        }

        // lda ($80) / and $289f / bne @bd1f
        uint16_t ptr_addr = read16(ram, 0x80);
        uint8_t val = ram[ptr_addr];
        if ((val & ram[0x289F]) != 0) {
            // @bd1f: lda #$01 / sta $de
            ram[0xDE] = 0x01;
            // lda $289d / bpl @bd3c
            if ((int8_t)ram[0x289D] >= 0) return;
        } else {
            // lda $289d / bpl @bd28
            if ((int8_t)ram[0x289D] < 0) {
                // stz $de / rts
                ram[0xDE] = 0;
                return;
            }
        }

    loop_continue:
        // @bd28: longa / clc / lda $80 / adc #$0080 / sta $80
        // Increments the indirect pointer by 0x80
        uint16_t ptr = read16(ram, 0x80);
        ptr += 0x0080;
        write16(ram, 0x80, ptr);

        // shorta0 / inx2 / cpx #$001a
        cpu->x += 2;
        if (cpu->x == 0x001A) break; // bne @bcff (loop if NOT 0x1A)

    } while (1);
}

// PITFALLS: 1 (DB=$BC), 6 (Mode A 16-bit used for ptr increment), 7 (Carry propagation in 8-bit ADC),
//            8 (mf=true inherited)
// HELPERS: GetAICondTarget_emu(snes), read16, write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xDD=1, 0x289E=1, 0x289F=1, 0x289D=1, 0x35D0=1
//   output_ram:  0xDE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xBC
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_00 ($BC:E8)