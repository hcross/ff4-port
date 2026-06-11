// AICond_02 – battle condition check
//   Reads an index from $289E, fetches a byte from the table at $35F3+index,
//   compares it with the target byte at $289F, and increments $DE if equal.
//
// Assumptions:
//   • Entry A is 8‑bit (mf = true), X is 16‑bit (xf = false) – default for
//     the battle module, so TAX zero‑extends the accumulator.
//   • Data bank = $7E (WRAM), direct page = $00 (DP = 0).
//
// No flags need to be preset because the routine starts with LDA.
// No sub‑routines are called.
//
// PITFALLS: 1 (DB must be $7E for $DE store), 8 (mode inheritance – use mf=true, xf=false)
static void AICond_02_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    uint8_t idx   = ram[0x289E];                 // load condition index
    uint8_t value = ram[0x35F3 + idx];           // table lookup
    uint8_t target = ram[0x289F];                // expected value

    if (value == target) {                       // cmp + bne inverted logic
        ram[0x00DE]++;                           // inc $DE  (WRAM address $7E:00DE)
    }
}

// HELPERS: none
// CONTRACT:
//   inputs_ram: 0x289E=1, 0x289F=1, 0x35F3..0x36F2=1   (table byte read)
//   output_ram: 0x00DE=1
//   entry_mode: mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none (routine does not test Z/N at entry)
REVERSED_FUNCTION: battle::AICond_02 ($BD:A9)