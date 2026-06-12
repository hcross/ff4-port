// This is not a routine but a jump table (address table) consisting of
// 20 entries of 16-bit pointers to AI target handlers.
// Since this is data, it is implemented as a static constant array.
static const uint16_t AITargetTbl[] = {
    0xBBB6, // AITarget_16 (Note: Addresses are relative to the bank in ASM, 
            // in C we typically represent the table and let the 
            // emulator/harness resolve the jump to the target)
    0xBBB8, // AITarget_17
    0xBBBA, // AITarget_18
    0xBBBC, // AITarget_19
    0xBBBE, // AITarget_1a
    0xBB C0, // AITarget_1b
    0xBB C2, // AITarget_1c
    0xBB C4, // AITarget_1d
    0xBB C6, // AITarget_1e
    0xBB C8, // AITarget_1f
    0xBB CA, // AITarget_20
    0xBB CC, // AITarget_21
    0xBB CE, // AITarget_22
    0xBB D0, // AITarget_23
    0xBB D2, // AITarget_24
    0xBB D4, // AITarget_25
    0xBB D6, // AITarget_26
    0xBB D8, // AITarget_27
    0xBB DA, // AITarget_28
    0xBB DC  // AITarget_29
};

// PITFALLS: None. This is a data table, not executable code.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xBB
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Data table, not a function)

REVERSED_FUNCTION: battle::AITargetTbl ($BB:B6)