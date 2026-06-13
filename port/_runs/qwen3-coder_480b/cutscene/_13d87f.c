#include "snes/snes.h"

// This function performs a 9-byte ROL (rotate left through carry) operation
// starting from $02 and then 8 bytes at $2B03,X through $2B0A,X.
// It assumes A is 8-bit and X is 16-bit, operating in WRAM bank $7E.
static void _13d87f_c(Snes *snes, uint16_t x) {
    uint8_t *ram = snes->ram;
    uint8_t carry = snes->cpu->c;

    // Process $2b0a,x through $2b03,x in descending order
    for (int offset = 0x0A; offset >= 0x03; offset--) {
        uint16_t addr = 0x2B00 + offset + x;
        uint16_t full_val = (uint16_t)(ram[addr] << 1) | carry;  // PITFALL 7: 8-bit ROL
        ram[addr] = full_val & 0xFF;
        carry = (full_val >> 8) & 1;
    }

    // Process $02 (single byte)
    uint16_t val = (uint16_t)(ram[0x02] << 1) | carry;  // PITFALL 7: 8-bit ROL
    ram[0x02] = val & 0xFF;
    snes->cpu->c = (val >> 8) & 1;  // Update carry flag from final ROL
}

// PITFALLS: 7 (ROL in 8-bit mode truncates to 8 bits, carry holds bit 8)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  0x02=1, 0x2B03=1, 0x2B04=1, 0x2B05=1, 0x2B06=1, 0x2B07=1, 0x2B08=1, 0x2B09=1, 0x2B0A=1
//   output_ram:  0x02=1, 0x2B03=1, 0x2B04=1, 0x2B05=1, 0x2B06=1, 0x2B07=1, 0x2B08=1, 0x2B09=1, 0x2B0A=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: c=auto
// REVERSED_FUNCTION: cutscene::_13d87f ($D8:7F)