// Logic:
// Checks if the current multiplier is exactly 2.
// If so, it checks if the target's element ($2721/2720) is weak to the 
// attack's element ($28A2).
// - If $2721 matches: multiplier becomes 8 (4x restored).
// - Else if $2720 matches: multiplier becomes 4 (2x restored).
// Otherwise, the multiplier remains unchanged.
static void CheckWeakElem_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
    if (ram[0x38FE] != 0x02) { // lda $38fe / cmp #$02 / bne @e155
        return;
    }

    // Check primary element weak match
    if ((ram[0x2721] & ram[0x28A2]) != 0) { // lda $2721 / and $28a2 / beq @e148
        ram[0x38FE] = 0x08; // sta $38fe
        return;
    }

    // Check secondary element weak match
    if ((ram[0x2720] & ram[0x28A2]) != 0) { // lda $2720 / and $28a2 / beq @e155
        ram[0x38FE] = 0x04; // sta $38fe
    }
}

// PITFALLS: 1 (DB=$7E assumed for battle logic)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x38FE=1, 0x2721=1, 0x2720=1, 0x28A2=1
//   output_ram:  0x38FE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::CheckWeakElem ($E1:33)