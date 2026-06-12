// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Calculate a summon attack value based on a provided parameter in A.
//   - If A < 0x3E: A = (A - 0x31) + 0x4D
//   - If A == 0x3F: A = 0x5D
//   - If A == 0x3E: A = (A - 0x31) + 0x4D
//   - If A > 0x3E and A != 0x3F: A = RandXA(2) + 0x3E, then (A - 0x31) + 0x4D
// Note: The logic for A=0x3E is handled by the fall-through of bcc @e071.
static void GetSummonAttack_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = (uint8_t)snes->cpu->a;

    if (a < 0x3E) { // cmp #$3e / bcc @e071
        // Fall through to @e071
    } else if (a == 0x3F) { // cmp #$3f / bne @e05f
        a = 0x5D; // lda #$5d / bra @e077
        ram[0x26D2] = a;
        ram[0x3584]++;
        return;
    } else { // A >= 0x3E and A != 0x3F (i.e., A is 0x3E or > 0x3F)
        // Note: if A == 0x3E, the asm actually executes cmp #$3e (not bcc), 
        // then cmp #$3f (bne taken), then enters the RandXA block.
        // Wait: looking at the asm:
        // @e053: cmp #$3e -> if A < 0x3e goto @e071
        // if A >= 0x3e: cmp #$3f -> if A != 0x3f goto @e05f
        // This means if A == 0x3e, it goes to @e05f.
        
        snes->cpu->x = 0;    // ldx #0
        snes->cpu->a = 2;     // lda #2
        uint8_t rand_val = (uint8_t)randxa_emu(snes); // jsr RandXA
        
        ram[0x26D4] = 0xF8;   // lda #$f8 / sta $26d4
        
        // a = rand_val + 0x3E (Pitfall 7: truncate to 8-bit)
        a = (uint8_t)(rand_val + 0x3E); // clc / adc #$3e
    }

    // @e071:
    // a = (a - 0x31) + 0x4D
    a = (uint8_t)(a - 0x31); // sec / sbc #$31
    a = (uint8_t)(a + 0x4D); // clc / adc #$4d

    // @e077:
    ram[0x26D2] = a;          // sta $26d2
    ram[0x3584]++;            // inc $3584
}

// PITFALLS: 7 (8-bit arithmetic truncation for ADC/SBC)
// HELPERS: randxa_emu(snes) — delegates RandXA @ $00:8379
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x26D2=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetSummonAttack ($E0:53)