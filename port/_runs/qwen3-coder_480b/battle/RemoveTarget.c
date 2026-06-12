// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: ram[$3554] determines target list, ram[$CE] bit 7 marks a target
// Logic:
//   if $3554 == 0:
//     clear bit (A & $7F) in $3550 (targets)
//   else:
//     clear bit (A & $7F) in $3523 (targets reflected onto)
static void RemoveTarget_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = ram[0x3554];
    uint8_t bit = ram[0xCE] & 0x7F;

    if (a != 0) {
        // lda $ce / and #$7f / tax
        // lda $3523 / jsr ClearBit / sta $3523
        uint8_t targets = ram[0x3523];
        uint8_t result = clear_bit_emu(snes, targets, bit);  // delegate ClearBit
        ram[0x3523] = result;
    } else {
        // lda $ce / and #$7f / tax
        // lda $3550 / jsr ClearBit / sta $3550
        uint8_t targets = ram[0x3550];
        uint8_t result = clear_bit_emu(snes, targets, bit);  // delegate ClearBit
        ram[0x3550] = result;
    }
}

// PITFALLS: 1 (DB=$7E required for WRAM access)
// HELPERS: clear_bit_emu(snes, value, bit_index) — delegates ClearBit @ $03:855A
// CONTRACT:
//   inputs_ram: 0x3554=1, 0xCE=1
//   output_ram: 0x3550=1, 0x3523=1
//   entry_mode: mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::RemoveTarget ($E0:30)