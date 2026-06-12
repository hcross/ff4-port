// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: cpu->a = summon_id (8-bit)
// Output: ram[$26D2] = attack_id (8-bit), ram[$3584] incremented
static void GetSummonAttack_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = snes->cpu->a;

    if (a >= 0x3E) {                    // cmp #$3e / bcc @e071
        if (a == 0x3F) {                // cmp #$3f / bne @e05f
            a = 0x5D;                   // lda #$5d
            goto store_result;          // bra @e077
        }
        // else: a > 0x3F
        snes->cpu->x = 0;               // ldx #0
        snes->cpu->a = 2;               // lda #2
        RandXA_emu(snes);               // jsr RandXA
        uint8_t rand_result = snes->cpu->a;
        ram[0x26D4] = 0xF8;             // sta $26d4
        a = (uint8_t)(rand_result + 0x3E); // adc #$3e (8-bit)
    } else {
        // a < 0x3E
        a -= 0x31;                      // sec / sbc #$31
        a += 0x4D;                      // clc / adc #$4d
    }

store_result:
    ram[0x26D2] = a;                    // sta $26d2
    ram[0x3584]++;                      // inc $3584
}

// PITFALLS: 1 (DB=$7E required for absolute stores), 6 (mode A is 8-bit),
// 7 (arithmetic truncation in 8-bit mode)
// HELPERS: RandXA_emu(snes) — delegates RandXA @ $03:8379
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x26D2=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetSummonAttack ($E0:53)