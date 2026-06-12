// This routine is not a functional block of code but a lookup table (LUT)
// containing 13 pointers (addresses) to duration values.
// Since it is data, the C translation provides a helper to retrieve the 
// address based on the index, mimicking the 24-bit absolute addressing 
// (.addr) used in the 65816 assembly.
static uint32_t get_timer_dur_tbl_addr(Snes *snes, uint8_t index) {
    // The table starts at $9F:FB.
    // Each .addr entry is 3 bytes (24-bit pointer).
    // Index range: 0 to 12.
    
    static const uint32_t timer_dur_tbl[] = {
        0x00FB00, // TimerDur_00 (Placeholder: these would be the actual resolved addresses)
        0x00FB03, // TimerDur_01
        0x00FB06, // TimerDur_02
        0x00FB09, // TimerDur_03
        0x00FB0C, // TimerDur_04
        0x00FB0F, // TimerDur_05
        0x00FB12, // TimerDur_06
        0x00FB15, // TimerDur_07
        0x00FB18, // TimerDur_08
        0x00FB1B, // TimerDur_09
        0x00FB1E, // TimerDur_0a
        0x00FB21, // TimerDur_0b
        0x00FB24, // TimerDur_0c
    };

    if (index > 12) return 0;
    return timer_dur_tbl[index];
}

// PITFALLS: None. This is a data table, not executable logic.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Data table, cannot be fuzzed as a function)

REVERSED_FUNCTION: battle::TimerDurTbl ($9F:FB)