#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: none (no registers or flags affect entry)
// Logic:
//   Check if a party member took HP damage (not MP damage or HP restore)
//   If so, increment $de (likely a counter or flag for AI condition)
static void AICond_0a_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = (ram[0xD2] - 5) * 2;  // sec / sbc #$05 / asl / tax
    uint16_t dmg = read16(ram, 0x34D4 + x); // 16-bit damage taken
    if (dmg == 0) return;              // ora $34d5,x / beq → skip
    uint8_t hi = ram[0x34D5 + x];
    if ((hi & 0xC0) != 0) return;      // and #$c0 / bne → skip
    ram[0xDE]++;                       // inc $de
}

// PITFALLS: 6 (assumed 8-bit A, 16-bit X from context), 7 (subtracted value
// must be truncated to 8-bit before *2 to match X register behavior)
// HELPERS: read16 for 16-bit memory access
// CONTRACT:
//   inputs_ram: 0xD2=1, 0x34D4=2
//   output_ram: 0xDE=1
//   entry_mode: mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: battle::AICond_0a ($BE:EC)