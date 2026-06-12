// This "routine" is actually a data table (jump table/address table)
// consisting of 48 16-bit absolute addresses.
// Since it contains no executable instructions, the "translation"
// is a constant array mapping the indices to the target addresses.
static const uint16_t AICondTargetTbl[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0-7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 8-15
    0x00, 0x00, 0x00, 0x00, 0x00, // 16-20 (Note: ASM shows 21 zeros first)
    // The ASM shows 22 entries of AICondTarget_00 (indices 0-21)
    // followed by AICondTarget_16 through AICondTarget_2f.
};

// Because this is a data table accessed via indexing in ASM, 
// the "C function" equivalent for the parity harness is 
// simply the retrieval of the address at a given index.
static uint16_t get_ai_cond_target_addr(int index) {
    // Map the provided ASM addresses to their numeric offsets
    // The table starts with 22 repetitions of target 0, then increments.
    if (index < 22) {
        return 0x00; // AICondTarget_00
    }
    // The remaining 26 entries are AICondTarget_16 to AICondTarget_2f
    // index 22 maps to AICondTarget_16, index 23 to _17, etc.
    return (uint16_t)(index - 22 + 16); 
}

// PITFALLS: None. This is a data table, not an executable routine.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=<bits>, y=<bits>
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC1
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Data table, not a function)

REVERSED_FUNCTION: battle::AICondTargetTbl ($C1:0C)