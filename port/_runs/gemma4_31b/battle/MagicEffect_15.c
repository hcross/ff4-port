// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   1. Calculate damage via CalcDmg (results stored in $a4-$a5)
//   2. Treat $a4-$a5 as a 16-bit value, add it to the accumulator at $270B
//   3. Clamp the result to the maximum value stored at $270D
//   4. Update $270B with the clamped sum
//   5. Set the high bits of $a5 (damage high byte) to 0xC0 (flagging)
static void MagicEffect_15_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // jsr CalcDmg (delegated)
    calc_dmg_emu(snes);

    // longa: switch to 16-bit A
    // clc / lda $a4 / adc $270b
    // The asm uses $a4 (damage lo) and $a5 (damage hi) as a 16-bit word
    uint16_t damage = read16(ram, 0xA4);
    uint16_t current_val = read16(ram, 0x270B);
    uint32_t sum = (uint32_t)damage + current_val;

    // cmp $270d / bcc @d9ff
    uint16_t max_val = read16(ram, 0x270D);
    if (sum >= max_val) { // bcc branches if sum < max_val (A < mem)
        sum = max_val;    // lda $270d
    }

    // sta $270b (writes 16-bit since A is longa)
    write16(ram, 0x270B, (uint16_t)sum);

    // shorta0: A = 0, then 8-bit mode
    // lda $a5 / ora #$c0 / sta $a5
    uint8_t a5_val = ram[0xA5];
    ram[0xA5] = (uint8_t)(a5_val | 0xC0);
}

// PITFALLS: 6 (Mode A 8-bit vs 16-bit: transition from CalcDmg 8-bit to 
// longa 16-bit for the summation and clamping), 3 (CMP/BCC inversion: 
// bcc branches if A < mem, so we clamp if A >= mem).
// HELPERS: calc_dmg_emu(snes) — delegates CalcDmg @ $03:C99F
//          read16/write16 — little-endian 16-bit accessors
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x270B=2, 0x270D=2
//   output_ram:  0x270B=2, 0x00A5=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_15 ($D9:EC)