// CmdTargetTbl is a 26-byte lookup table indexed by command ID.
// It maps each command to a targeting mode used in battle.
// The table is read directly by other routines via ram[0x7E00 + offset].

static void CmdTargetTbl_c(Snes *snes) {
    // This is a data-only routine — no execution logic.
    // The table is embedded in the function body as a static array
    // and copied into WRAM at the expected location.
    static const uint8_t table[26] = {
        0x50, 0x00, 0x00, 0x00, 0x00, 0x60, 0x58, 0x60,
        0x50, 0x00, 0x20, 0x20, 0x50, 0x50, 0x60, 0x00,
        0x60, 0x00, 0x60, 0x18, 0x58, 0x60, 0x50, 0x50,
        0x00, 0x28
    };
    uint8_t *ram = snes->ram;
    for (int i = 0; i < 26; i++) {
        ram[0x7E00 + i] = table[i];
    }
}

// PITFALLS: none (data-only)
// HELPERS: none (no code)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x7E00=1, 0x7E01=1, ..., 0x7E19=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none
REVERSED_FUNCTION: battle::CmdTargetTbl ($FD:C3)