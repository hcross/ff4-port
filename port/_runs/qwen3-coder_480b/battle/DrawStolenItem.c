// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// Entry: Y = item index to draw (passed via tya)
// Logic:
//   Store Y in $01, then call ExecBtlGfx with A=$06 to draw item text
static void DrawStolenItem_c(Snes *snes, uint16_t item_index) {
    uint8_t *ram = snes->ram;
    ram[0x01] = (uint8_t)item_index;     // tya / sta $01 (truncates to 8-bit)
    exec_btl_gfx_emu(snes, 0x06);        // lda #$06 / jsr ExecBtlGfx
}

// PITFALLS: 1 (DB must be $7E for WRAM access), 6 (A is 8-bit on entry)
// HELPERS: exec_btl_gfx_emu(snes, arg) — delegates ExecBtlGfx @ $03:8085
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=16
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::DrawStolenItem ($E2:D2)