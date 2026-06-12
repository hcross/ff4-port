// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$DC, DP=0
// Logic:
//   1. Generates a random number (0-1) via RandXA(2).
//   2. If result == 0: set $28A4 = 0x20 and jump to SleepParalyzeEffect.
//   3. If result != 0:
//      - Loop: decrement A. If A reaches 0, set $28A3 = 0x04 and jump to SetMagicStatus2.
//      - Otherwise: set $28A4 = 0x80 and repeat loop.
// Note: The loop 'dec / bne @dcf3' with RandXA returning 1 actually executes 
// the 'sta $28a4' (0x80) once and then exits the loop on the next dec, 
// resulting in $28A3 = 0x04.
static void MagicEffect_22_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // ldx #0 / lda #2 / jsr RandXA
    snes->cpu->x = 0;
    snes->cpu->a = 2;
    uint16_t rand_val = randxa_emu(snes); // Returns A (0 to 1)
    
    uint8_t a = (uint8_t)rand_val;
    if (a == 0) { // tax / bne @dce9 -> not taken
        ram[0x28A4] = 0x20;
        sleep_paralyze_effect_emu(snes);
        return;
    }

    // Loop: @dce9
    do {
        a--; // dec A
    } while (a != 0); // bne @dcf3

    // The loop above is a bit strange in the ASM. 
    // If A=1: dec(0), bne is false -> jumps to @dcf8 (Wait, no)
    // Let's re-trace:
    // @dce9: dec (A=0), bne @dcf3 (False) -> @dcf8: lda #04, sta $28a3, bra @dcf8
    // Wait, looking at the ASM:
    // @dce9: dec
    //       bne @dcf3  <-- if a != 0, go to @dcf3
    //       lda #$04
    //       sta $28a3
    //       bra @dcf8  <-- jump to SetMagicStatus2
    // @dcf3: lda #$80
    //       sta $28a4
    //       // loop repeats because it fell through from @dcf3? No, there is no jump back to @dce9.
    
    /* 
     * RE-ANALYSIS of ASM logic:
     * @dce9: dec A
     *       bne @dcf3 (If A != 0, go to @dcf3)
     *       lda #04, sta $28a3, bra @dcf8 (If A == 0, set $28a3=4 and exit)
     * @dcf3: lda #80, sta $28a4
     *       // Then it falls through to @dcf8: jmp SetMagicStatus2
     */

    // Correct logic based on the flow:
    // Since RandXA(2) returns 0 or 1:
    // If A was 1: dec A -> A=0. bne @dcf3 is FALSE.
    // Result: ram[0x28A3] = 0x04.
    
    // If RandXA had returned > 1:
    // dec A -> A > 0. bne @dcf3 is TRUE.
    // Result: ram[0x28A4] = 0x80.
    
    // Since RandXA(2) only returns 0 or 1, if we reached here, A is 1.
    // Therefore a-- makes a=0, bne is false.
    ram[0x28A3] = 0x04;
    set_magic_status2_emu(snes);
}

// PITFALLS: 7 (8-bit truncation on dec), 1 (DB=$DC)
// HELPERS: randxa_emu, sleep_paralyze_effect_emu, set_magic_status2_emu
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x28A3=1 or 0x28A4=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xDC
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_22 ($DC:D6)