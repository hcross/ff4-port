// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$FE, DP=0
// This "routine" is actually a data table (script) used by the Auto-Battle system.
// It defines a sequence of actions: [Command: Fight], [Magic: Flame], [End].
// The "translation" returns a pointer to the data or writes the data to a 
// target buffer depending on how the caller invokes this memory range.
static void AutoBattle_0005_c(Snes *snes, uint8_t *target_buffer) {
    // The data at $FE:46 is a sequence of bytes:
    // 0xC0, 0x00 -> Action: Fight ( Command 0x00 )
    // 0x00, 0x42 -> Action: Magic ( Command 0x00, Value 0x42 [Flame] )
    // 0xFF       -> End of script
    
    uint8_t script[] = { 0xC0, 0x00, 0x00, 0x42, 0xFF };
    
    for (int i = 0; i < 5; i++) {
        target_buffer[i] = script[i];
    }
}

// PITFALLS: None (Static data translation)
// HELPERS: None
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none (writes to provided target_buffer)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFE
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes

REVERSED_FUNCTION: battle::AutoBattle_0005 ($FE:46)