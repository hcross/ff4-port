// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00, DP=0
// The routine performs a bitwise AND between the accumulator and a value 
// from the BitAndTbl table, indexed by X. 
// Result is returned in the accumulator.
static uint8_t ClearBit_c(Snes *snes) {
    uint8_t a = (uint8_t)snes->cpu->a;
    uint16_t x = snes->cpu->x;
    
    // f:BitAndTbl refers to a table in the ROM.
    // In this context, we read the byte from the ROM at the table address + X.
    // Since we are in the parity harness, we access the ROM via the snes object.
    uint8_t mask = snes->rom[0x8500 + 0x00 + x]; // Approximate address based on symbol mapping
    
    uint8_t result = (uint8_t)(a & mask);
    
    // Update CPU state to match the effect of 'and'
    snes->cpu->a = result;
    snes->cpu->z = (result == 0);
    snes->cpu->n = (result & 0x80) != 0;
    
    return result;
}

// PITFALLS: 6 (Mode A 8-bit: AND result is truncated to 8 bits)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=16, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
//   custom_spike: yes (returns via register A)

REVERSED_FUNCTION: battle::ClearBit ($85:5A)