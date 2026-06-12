// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// Logic: 
//   The routine takes the item ID in Y, saves it to RAM $01 (DP offset),
//   then triggers the battle graphics engine to draw the inventory item text
//   using command 0x06.
static void DrawStolenItem_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Routine preserves X and Y via phx/phy...ply/plx
    // The item ID is passed in Y
    uint16_t item_id = snes->cpu->y;
    
    // sta $01 (DP=0)
    // Note: Since Y is typically 16-bit here (xf=0) but 'sta' is 8-bit,
    // only the low byte of Y is stored to $01.
    ram[0x01] = (uint8_t)(item_id & 0xFF);
    
    // lda #$06 (Battle graphics command for draw inventory item text)
    snes->cpu->a = 0x06;
    
    // jsr ExecBtlGfx (delegated)
    exec_btl_gfx_emu(snes);
}

// PITFALLS: None applicable (simple register-to-RAM transfer and delegation).
// HELPERS: exec_btl_gfx_emu(snes) — delegates ExecBtlGfx @ 8085
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=16
//   inputs_ram:  none
//   output_ram:  0x0001=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto

REVERSED_FUNCTION: battle::DrawStolenItem ($E2:D2)