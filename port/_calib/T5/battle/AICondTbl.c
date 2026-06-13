#include "snes/snes.h"

// AICondTbl is a jump table of 12 16-bit addresses (AICond_00..AICond_0b)
// stored at ROM $C0:F4.  This C function reads the table entry for a given
// index (0..11) and returns the corresponding 16-bit address.
//
// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0.
// The caller provides the index in the low byte of A; the function returns
// the full 16-bit address in A (the harness captures the return value).
static uint16_t AICondTbl_c(Snes *snes, uint8_t index) {
    // ROM address = $C0:F4 + index*2  (little-endian 16-bit entries)
    uint32_t rom_addr = 0xC0F4u + (uint32_t)index * 2u;
    uint8_t lo = snes->rom[rom_addr];
    uint8_t hi = snes->rom[rom_addr + 1];
    return (uint16_t)(lo | (hi << 8));
}

// PITFALLS: none (no branches, no flag dependencies, no mode-sensitive
//          arithmetic — pure data lookup)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8 (index 0..11)
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: battle::AICondTbl ($C0:F4)