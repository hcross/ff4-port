// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Input: none (hardcoded to $FF)
// Output: returned in A (8-bit)
static uint8_t Rand_c(Snes *snes) {
    // clr_ax → tdc / tax (A = X = 0, since DP=0)
    uint16_t a = 0;
    uint16_t x = 0;
    uint8_t arg = 0xFF;

    // jsr RandXA (expects A and X as inputs, returns in A)
    // Set up CPU state for delegated call:
    Cpu *c = snes->cpu;
    c->a = (c->a & 0xFF00) | (a & 0xFF);  // preserve upper byte B
    c->x = x;
    c->y = 0;  // not used, but zeroed for cleanliness
    c->dp = 0;
    c->db = 0x7E;
    c->mf = true;   // A 8-bit
    c->xf = false;  // X/Y 16-bit
    c->z = (arg == 0);
    c->n = (arg & 0x80) != 0;

    run_emulated_func(snes, 0x8379u);  // RandXA
    return (uint8_t)(snes->cpu->a & 0xFF);
}

// PITFALLS: 1 (DB=$7E), 2 (Z/N flags for jsr target), 9 (upper byte B preserved)
// HELPERS: run_emulated_func (for RandXA delegation)
// CONTRACT:
//   inputs_reg:  a=8, x=16, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::Rand ($85:93)