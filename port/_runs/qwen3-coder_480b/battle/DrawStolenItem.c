// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// Entry: Y = item index to draw (passed via tya)
// Logic:
//   Store Y in $01, then call ExecBtlGfx with A=$06 to draw item text
static void DrawStolenItem_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *c = snes->cpu;

    // Push X and Y (emulated via stack tracking in harness)
    // tya / sta $01
    ram[0x01] = (uint8_t)(c->y & 0xFF);

    // lda #$06 / jsr ExecBtlGfx
    c->a = 0x06;
    c->mf = true;  // ensure 8-bit A for ExecBtlGfx
    ExecBtlGfx_emu(snes);
}

// PITFALLS: 1 (DB must be $7E for WRAM access), 6 (A is 8-bit on entry)
// HELPERS: ExecBtlGfx_emu(snes) — delegates ExecBtlGfx @ $03:8085
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=16
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::DrawStolenItem ($E2:D2)