// This "routine" is actually a data table (lookup table) used by the battle 
// engine to determine target properties/types based on a command index.
// Since it is pure data, the "translation" is a function that returns 
// the byte at the given index within the table.
static uint8_t CmdTargetTbl_c(Snes *snes, uint8_t index) {
    // Table located at $FD:C3
    static const uint8_t table[] = {
        0x50, 0x00, 0x00, 0x00, 0x00, 0x60, 0x58, 0x60, 
        0x50, 0x00, 0x20, 0x20, 0x50, 0x50, 0x60, 0x00,
        0x60, 0x00, 0x60, 0x18, 0x58, 0x60, 0x50, 0x50, 
        0x00, 0x28
    };

    if (index >= (sizeof(table) / sizeof(uint8_t))) {
        return 0; // Bounds check for safety, though asm wouldn't have one
    }
    return table[index];
}

// PITFALLS: None. This is a static data table, not executable code.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFD
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (This is a data table, parity is checked via read access)

REVERSED_FUNCTION: battle::CmdTargetTbl ($FD:C3)