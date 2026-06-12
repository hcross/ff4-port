// Entry mode: A 8-bit (inherited mf=1), X 16-bit (inherited xf=0)
// No input registers; returns random monster index in A (0-6)
static void RandMonster_c(Snes *snes) {
    // ldx #0
    snes->cpu->x = 0;
    // lda #7
    snes->cpu->a = 7;
    // jsr RandXA
    rand_xa_emu(snes);
    // rts (return value in A)
}

// PITFALLS: 1 (DB must be $7E for WRAM access if any — but this routine
// uses no WRAM), 2 (no flags needed — no branch on entry)
// HELPERS: rand_xa_emu(snes) — delegates RandXA @ $03:8379
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::RandMonster ($03:8579)