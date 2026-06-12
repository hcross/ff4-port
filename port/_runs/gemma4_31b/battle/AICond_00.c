// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$BC (Bank), DP=0
// Purpose: Iterates through a set of AI conditions to determine a status value stored in $DE.
// The loop processes 26 entries (0x1a) starting from a base pointer, checking 
// condition masks and flags.
static void AICond_00_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // jsr GetAICondTarget
    get_ai_cond_target_emu(snes);

    // lda $dd / beq @bd3c
    if (ram[0xDD] == 0) return;

    // Construct initial address in $80/$81 (indirect pointer)
    // clc / lda #$03 / adc $289e / sta $80
    uint8_t addr_lo = (uint8_t)(0x03 + ram[0x289E]);
    ram[0x80] = addr_lo;
    // lda #$20 / adc #$00 / sta $81
    uint8_t addr_hi = (uint8_t)(0x20 + 0); // adc #0 preserves carry from previous op
    ram[0x81] = addr_hi;

    // clr_ax (A=X=D=0 given DP=0)
    cpu->a = 0;
    cpu->x = 0;

    do {
        // lda $35d0,x / sta $a9
        // lda $35d1,x / sta $aa
        // Mode A is 8-bit, so we access byte by byte. 
        // X is 16-bit, indexing into the range starting at $35D0.
        uint16_t offset = cpu->x;
        ram[0xA9] = ram[0x35D0 + offset];
        ram[0xAA] = ram[0x35D1 + offset];

        // ldy $a9 / cpy #$ffff
        // Since Y is 16-bit, loading 8-bit $a9 zero-extends it.
        uint16_t y = ram[0xA9];
        if (y == 0xFFFF) { // beq @bd28
            goto loop_end;
        }

        // lda ($80) / and $289f / bne @bd1f
        uint8_t ptr_val = ram[read16(ram, 0x80)];
        if ((ptr_val & ram[0x289F]) != 0) {
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

        loop_end:
        // @bd28: longa / clc / lda $80 / adc #$0080 / sta $80
        // Note: the ASM does 'lda $80' then 'adc #$0080' in 16-bit mode.
        // This means it loads the 16-bit word at $80 and adds 0x80.
        uint16_t ptr = read16(ram, 0x80);
        ptr += 0x80;
        write16(ram, 0x80, ptr);

        // shorta0 / inx2 / cpx #$001a
        cpu->x += 2;
        if (cpu->x != 0x001A) {
            continue; // bne @bcff
        } else {
            break;
        }
    } while (1);
}

// PITFALLS: 1 (DB=$BC), 6 (Mode A oscillation between 8 and 16 bit), 
//            7 (8-bit truncation on ADC/AND), 8 (mf=true inherited for battle)
// HELPERS: get_ai_cond_target_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xDD=1, 0x289E=1, 0x289F=1, 0x289D=1, 0x35D0=1
//   output_ram:  0xDE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xBC
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_00 ($BC:E8)