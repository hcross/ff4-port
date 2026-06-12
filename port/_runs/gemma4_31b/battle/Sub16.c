// Performs 16-bit subtraction of two values stored in WRAM.
// reads: ram[0x395E] (minuend), ram[0x3960] (subtrahend)
// writes: ram[0x3962] = result
static void Sub16_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // longa: A is now 16-bit
    // sec: carry flag set (essential for SBC)
    uint16_t a = read16(ram, 0x395E);
    uint16_t b = read16(ram, 0x3960);
    
    // sbc $3960 (with Carry set) is equivalent to (a - b)
    uint16_t result = (uint16_t)(a - b);
    
    write16(ram, 0x3962, result);
    
    // shorta0: clr_a / shorta
    // In a parity harness, the register state is usually checked.
    // We simulate the side-effects of shorta0 by updating CPU state.
    snes->cpu->a = snes->cpu->dp; // clr_a: transfer DP to A
    snes->cpu->mf = true;         // shorta: A 8-bit
}

// PITFALLS: 6 (Mode A 16-bit: uses read16/write16 for 2-byte operations)
// HELPERS: read16, write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x395E=2, 0x3960=2
//   output_ram:  0x3962=2
//   entry_mode:  mf=true, xf=false, dp=0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::Sub16 ($84:FC)