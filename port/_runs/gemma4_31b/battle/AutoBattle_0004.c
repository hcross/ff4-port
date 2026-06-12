// This routine is a data table (script) containing a sequence of magic values.
// Each magic value consists of 2 bytes (16-bit LE).
// The sequence is terminated by a 0xFF byte.
// Since this is a data block and not executable code, the "translation" 
// provides a way to read these values from the ROM/RAM context.
static void AutoBattle_0004_c(Snes *snes) {
    // This function is a data table. In the C reimplementation, 
    // the logic that consumes this table will read from the 
    // ROM area at $FE:3B.
}

// PITFALLS: None. This is a static data table.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFE
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

REVERSED_FUNCTION: battle::AutoBattle_0004 ($FE:3B)