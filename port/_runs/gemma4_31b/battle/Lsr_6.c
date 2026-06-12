// Entry mode: A 8-bit (mf=true), X 16-bit (xf=false), DB=$7E, DP=0
// This routine performs 6 consecutive logical shift rights (LSR) on the 
// accumulator. This is equivalent to dividing by 2^6 (64).
//
// The result is stored back in the accumulator for the caller.
static uint16_t Lsr_6_c(Snes *snes) {
    uint8_t a = (uint8_t)snes->cpu->a;
    
    // Perform 6 shifts. Pitfall 7: ensure we truncate to 8-bit 
    // to match 65816 8-bit mode behavior.
    for (int i = 0; i < 6; i++) {
        a = (uint8_t)(a >> 1);
    }
    
    snes->cpu->a = a;
    return a;
}

// PITFALLS: 6 (Mode A 8-bit), 7 (Arithmetic truncation: forced (uint8_t)
// cast to ensure overflow/carry behaves like 8-bit register)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes (output is in register A, not RAM)

REVERSED_FUNCTION: battle::Lsr_6 ($84:82)