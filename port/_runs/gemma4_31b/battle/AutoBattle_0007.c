// This is not an executable routine but a data table (script) used by the 
// AutoBattle AI engine to determine action sequences for specific characters.
// Each action is [Command, Value], terminated by 0xFF.
static void AutoBattle_0007_c(Snes *snes) {
    // This function is a placeholder for the parity harness as the 
    // original ASM is static data located at $FE:5D.
    // The data is accessed by the AI engine via absolute addressing.
    (void)snes;
}

// The script data corresponds to the following memory layout in WRAM/ROM:
// $FE:5D: 00 2F (Magic: Meteo)
// $FE:5F: 00 0C (Magic: Dispel)
// $FE:61: FF     (End)
// $FE:62: 00 2F (Magic: Meteo)
// $FE:64: 01 C8 (Item: Crystal)
// $FE:66: FF     (End)

// PITFALLS: None.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  n/a
//   entry_flags: n/a
// CUSTOM_SPIKE: yes

REVERSED_FUNCTION: battle::AutoBattle_0007 ($FE:5D)