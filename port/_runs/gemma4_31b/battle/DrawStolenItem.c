// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   Saves the item ID from Y to RAM $01 and calls the battle graphics 
//   engine (ExecBtlGfx) with command 0x06 to draw the item text.
static void DrawStolenItem_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // phx / phy are implicit in C as we use local variables or 
    // read from snes->cpu and don't modify the registers permanently.
    uint16_t y_val = snes->cpu->y;

    // tya / sta $01 (DP=0)
    // In 8-bit mode, only the low byte of Y is transferred to A and stored.
    ram[0x01] = (uint8_t)(y_val & 0xFF);

    // lda #$06
    snes->cpu->a = 0x06;

    // jsr ExecBtlGfx (delegated)
    ExecBtlGfx_emu(snes);

    // ply / plx are implicit.
}

// PITFALLS: None applicable.
// HELPERS: ExecBtlGfx_emu(snes) — delegates ExecBtlGfx @ 8085
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=16
//   inputs_ram:  none
//   output_ram:  0x0001=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto

REVERSED_FUNCTION: battle::DrawStolenItem ($E2:D2)