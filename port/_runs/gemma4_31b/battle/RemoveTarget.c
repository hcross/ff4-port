// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Removes the current target from the active target bitmask.
// Depending on whether a specific target flag ($3554) is set, it clears the bit 
// in either the primary target list ($3550) or the reflected target list ($3523).
static void RemoveTarget_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if (ram[0x3554] == 0) { // lda $3554 / bne @e044
        // Clear bit in primary target list ($3550)
        uint8_t bit = ram[0xCE] & 0x7F; // lda $ce / and #$7f
        snes->cpu->x = bit;             // tax
        snes->cpu->a = ram[0x3550];     // lda $3550
        clear_bit_emu(snes);            // jsr ClearBit
        ram[0x3550] = (uint8_t)snes->cpu->a; // sta $3550
    } else {
        // Clear bit in reflected target list ($3523)
        uint8_t bit = ram[0xCE] & 0x7F; // lda $ce / and #$7f
        snes->cpu->x = bit;             // tax
        snes->cpu->a = ram[0x3523];     // lda $3523
        clear_bit_emu(snes);            // jsr ClearBit
        ram[0x3523] = (uint8_t)snes->cpu->a; // sta $3523
    }
}

// PITFALLS: 1 (DB=$7E assumed for battle module), 6 (A 8-bit for RAM access)
// HELPERS: clear_bit_emu(snes) — delegates ClearBit @ $00:855A
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3554=1, 0xCE=1, 0x3550=1, 0x3523=1
//   output_ram: 0x3550=1 (if $3554==0) or 0x3523=1 (if $3554!=0)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::RemoveTarget ($E0:30)