// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$9E, DP=0
// Logic:
//   Calculates the duration of a timer effect based on an item property.
//   1. Multiplies ram[$397B] by 6 using Mult8.
//   2. Uses the result as an index into the ItemProp table to fetch the delay.
//   3. The result is left in A. Note: in vanilla FFIV, these properties are usually 0.
static void TimerDur_0b_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $397b / sta $df
    ram[0xDF] = ram[0x397B];
    
    // lda #$06 / sta $e1
    ram[0xE1] = 0x06;

    // jsr Mult8
    // Mult8 is delegated. It typically reads $DF and $E1 and writes result to $E3.
    mult8_emu(snes);

    // ldx $e3
    uint16_t index = ram[0xE3];

    // lda f:ItemProp,x
    // Mapping the ItemProp table (mapped to memory). 
    // In this context, 'f' refers to the ROM/PRG bank mapping.
    // We use the snes instance to access the ROM data at the ItemProp offset.
    // Assuming ItemProp is a known constant address in the ROM.
    uint8_t delay = snes->rom[0xItemProp + index]; 
    
    // The routine ends with a branch (bra _9eab), but for a C implementation 
    // of a standalone routine, we represent the result in A.
    snes->cpu->a = delay;
}

// PITFALLS: 1 (DB=$9E used for DP accesses), 8 (Inherited mf=true for 8-bit A)
// HELPERS: mult8_emu(snes) — delegates Mult8 @ $83E0
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x397B=1, 0xItemProp=1 (array)
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x9E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Requires ROM mapping for ItemProp table)

REVERSED_FUNCTION: battle::TimerDur_0b ($9E:85)