// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Calculate summon attack power.
//   - If A < 0x3E: Result is (A - 0x31) + 0x4D
//   - If A == 0x3F: Result is 0x5D
//   - If A >= 0x3E and A != 0x3F: Result is (RandXA(2) + 0x3E - 0x31) + 0x4D
static void GetSummonAttack_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = (uint8_t)snes->cpu->a;

    if (a < 0x3E) { // cmp #$3e / bcc @e071
        // Jump to @e071 logic
    } else if (a == 0x3F) { // cmp #$3f / bne @e05f
        a = 0x5D; // lda #$5d / bra @e077
        ram[0x26D2] = a;
        ram[0x3584]++;
        return;
    } else { // A >= 0x3E and A != 0x3F
        snes->cpu->x = 0;    // ldx #0
        snes->cpu->a = 2;     // lda #2
        RandXA_emu(snes);     // jsr RandXA
        
        uint8_t rand_res = (uint8_t)snes->cpu->a; // Result of RandXA is in A
        ram[0x26D4] = 0xF8;   // lda #$f8 / sta $26d4
        
        // clc / adc #$3e
        a = (uint8_t)(rand_res + 0x3E); // Pitfall 7: 8-bit truncation
    }

    // @e071:
    // sec / sbc #$31
    a = (uint8_t)(a - 0x31); // Pitfall 7: 8-bit truncation
    // clc / adc #$4d
    a = (uint8_t)(a + 0x4D); // Pitfall 7: 8-bit truncation

    // @e077:
    ram[0x26D2] = a;          // sta $26d2
    ram[0x3584]++;            // inc $3584
}

// PITFALLS: 7 (8-bit arithmetic truncation for ADC/SBC)
// HELPERS: RandXA_emu(snes) — delegates RandXA @ $00:8379
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x26D2=1, 0x26D4=1, 0x3584=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetSummonAttack ($E0:53)