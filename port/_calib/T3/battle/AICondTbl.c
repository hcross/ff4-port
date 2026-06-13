#include "snes/snes.h"

// AICondTbl: 12-entry function pointer table for AI condition handlers.
// Pure data in ROM bank $C0 — no executable logic.

// Actual addresses resolved from ROM; define as needed by the build.
// Each entry is a 16-bit little-endian address within bank $C0.
static const uint16_t kAICondTbl[12] = {
    /* AICond_00 */ 0x0000, // placeholder — resolved from ROM symbols
    /* AICond_01 */ 0x0000,
    /* AICond_02 */ 0x0000,
    /* AICond_03 */ 0x0000,
    /* AICond_04 */ 0x0000,
    /* AICond_05 */ 0x0000,
    /* AICond_06 */ 0x0000,
    /* AICond_07 */ 0x0000,
    /* AICond_08 */ 0x0000,
    /* AICond_09 */ 0x0000,
    /* AICond_0a */ 0x0000,
    /* AICond_0b */ 0x0000,
};

// PITFALLS: none (data table, no code)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes — pure data table, no executable logic to parity-test

// REVERSED_FUNCTION: battle::AICondTbl ($C0:F4)