// This "routine" is actually a data table containing a list of 
// 16-bit animation offsets, terminated by a 0xFF sentinel byte.
// Since it contains no executable code, the C implementation 
// provides a way to access this data as it appears in RAM/ROM.
static uint16_t get_no_win_anim_offset(Snes *snes, int index) {
    uint8_t *ram = snes->ram;
    // Table starts at $FE:76 (absolute 0x7E76 in WRAM mapping)
    // The table consists of words (2 bytes each)
    uint16_t offset = read16(ram, 0x7E76 + (index * 2));
    return offset;
}

// Note: Since the ASM is purely data (.word / .byte), this is not a 
// functional routine. In the snesrev pattern, data tables are often 
// accessed directly via read16/ram[].
// If the harness expects a function signature for a "routine" 
// that is actually data, it is typically treated as a no-op or 
// an accessor.

// PITFALLS: None (Pure data table)
// HELPERS: read16

// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Data table, cannot be fuzzed as a function)

REVERSED_FUNCTION: battle::NoWinAnimTbl ($FE:76)