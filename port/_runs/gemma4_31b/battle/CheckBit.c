// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x0 (ROM lookup), DP=0
// Logic: Performs a bitwise AND between the accumulator and a value from 
// the BitOrTbl table, indexed by X. The result remains in the accumulator.
static void CheckBit_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    
    // The asm uses 'and f:BitOrTbl,x'. 
    // BitOrTbl is a table in the ROM. In the harness, ROM is mapped 
    // such that we can access it via the snes instance.
    // We assume the harness provides access to ROM labels via a pointer or 
    // that snes->rom is accessible.
    
    uint8_t table_val = snes->rom[BIT_OR_TBL + cpu->x];
    
    // Pitfall 7: Ensure 8-bit truncation for AND operation
    cpu->a = (uint8_t)(cpu->a & table_val);
    
    // Update flags Z and N based on 8-bit result
    cpu->z = (cpu->a == 0);
    cpu->n = (cpu->a & 0x80) != 0;
}

// PITFALLS: 7 (Explicit 8-bit truncation and flag update for AND)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=16, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
//   custom_spike: yes (output is in register A, not RAM)

REVERSED_FUNCTION: battle::CheckBit ($85:64)