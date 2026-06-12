// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   Stores the target bitmask in ram[$34C5].
//   Calls CountBits to determine the number of set bits (result in X).
//   If X > 1, the 0x40 bit is set in the target status byte at ram[$34C4]
//   to indicate a multi-target attack.
static void SetTargets_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // sta $34c5
    ram[0x34C5] = (uint8_t)snes->cpu->a;

    // jsr CountBits (result stored in snes->cpu->x)
    CountBits_emu(snes);

    // dex
    snes->cpu->x--;

    // beq @b19f: branch if X was 1 (now 0), meaning single target
    if (snes->cpu->x == 0) {
        return;
    }

    // multi-target path: lda $34c4 / ora #$40 / sta $34c4
    ram[0x34C4] |= 0x40;
}

// PITFALLS: 8 (Inherited mode: mf=true, xf=false for battle module)
// HELPERS: CountBits_emu(snes) — delegates CountBits @ $850C
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x34C4=1
//   output_ram:  0x34C5=1, 0x34C4=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::SetTargets ($B1:8E)