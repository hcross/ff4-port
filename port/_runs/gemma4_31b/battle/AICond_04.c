// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   1. Initializes a counter at ram[0xA9] = 0.
//   2. Loops through 3 entries in a table starting at $29AD.
//   3. If ram[0x289F] matches the table entry, it checks a secondary condition.
//   4. Secondary check: if ram[0x29CA + offset] is non-zero and matches ram[0x29CD], counter += 2.
//   5. If loop ends without match or secondary check fails, counter is incremented once (if loop finished).
//   6. Finally, compares counter with ram[0x289E]; if equal, increments ram[0xDE].
static void AICond_04_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // clr_ax / stx $a9 (X is 16-bit, but DP=0, so X=0)
    uint8_t counter = 0; 
    ram[0xA9] = counter;

    uint8_t target_id = ram[0x289F];
    bool matched = false;

    for (uint16_t x = 0; x < 3; x++) { // @bdf4: lda $289f / cmp $29ad,x
        if (target_id == ram[0x29AD + x]) {
            matched = true;
            // @be06: lda $29ca,x
            uint8_t val_ca = ram[0x29CA + x];
            if (val_ca == 0) {
                counter++; // beq @be02
            } else if (val_ca == ram[0x29CD]) {
                counter += 2; // cmp $29cd / bne @be14 / inc $a9 / inc $a9
            } else {
                // bne @be14: no increment
            }
            break; // beq @be06
        }
    }

    if (!matched) {
        counter++; // @be02: inc $a9
    }

    // @be14: lda $289e / cmp $a9 / bne @be1d
    if (ram[0x289E] == counter) {
        ram[0xDE]++; // inc $de
    }
}

// PITFALLS: 1 (DB=$7E required), 5 (clr_ax is tdc/tax), 8 (Inherited mf=true, xf=false)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x289F=1, 0x289E=1, 0x29AD=3, 0x29CA=3, 0x29CD=1
//   output_ram:  0xDE=1
//   entry_mode:  mf=true, xf=false, dp=0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_04 ($BD:F0)