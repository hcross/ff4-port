// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   1. Loads 16-bit value from $2707 into $3945-$3946 (dividend)
//   2. Sets divisor to 3 at $3947
//   3. Calls Div16 to compute quotient ($3949-$394A)
//   4. Clamps the quotient to a maximum of $270F
//   5. Stores the result in $A4-$A5, forcing the high byte's MSB to 1 (ORA #$80)
static void MagicEffect_2c_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Load dividend from $2707 (16-bit)
    ram[0x3945] = ram[0x2707];
    ram[0x3946] = ram[0x2708];

    // Divisor = 3
    write16(ram, 0x3947, 3);

    div16_emu(snes); // jsr Div16 (delegated)

    // Result is in $3949 (lo), $394A (hi)
    uint16_t quotient = read16(ram, 0x3949);

    // cpx #$270f / bcc @dd88 : if (quotient < 0x270F) skip clamping
    if (quotient > 0x270F) {
        quotient = 0x270F;
    }

    // Store clamped quotient back to $3949 (though not strictly needed for final store)
    write16(ram, 0x3949, quotient);

    // Store result to $A4 (low byte)
    ram[0xA4] = ram[0x3949];

    // Store result to $A5 (high byte) with bit 7 set (ORA #$80)
    ram[0xA5] = ram[0x394A] | 0x80;
}

// PITFALLS: 1 (DB=$7E required), 8 (Inherited mf=true, xf=false for battle module)
// HELPERS: div16_emu(snes) — delegates Div16 @ $03:85A3
//          read16/write16 — little-endian 16-bit accessors over ram[]

// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2707=2
//   output_ram:  0x00A4=1, 0x00A5=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_2c ($DD:65)