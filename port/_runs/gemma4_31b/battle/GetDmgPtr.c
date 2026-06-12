// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine converts a character index/type into a memory offset (pointer).
// If the input A is negative (bit 7 set), it is treated as a special 
// index that is masked and offset by 5 before being shifted.
static void GetDmgPtr_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = (uint8_t)snes->cpu->a;

    ram[0xA9] = a; // sta $a9

    if ((int8_t)a >= 0) { // bpl @ca6b
        // Branch taken: positive character index
    } else {
        // Branch not taken: special index processing
        a &= 0x7F;       // and #$7f
        a = (uint8_t)(a + 5); // clc / adc #$05 (Pitfall 7: truncate to 8-bit)
    }

    a = (uint8_t)(a << 1); // asl (Pitfall 7: truncate to 8-bit)
    
    snes->cpu->a = a;   // Result in A
    snes->cpu->x = a;   // tax
}

// PITFALLS: 7 (8-bit arithmetic truncation: both ADC and ASL wrapped in uint8_t 
// to prevent 16-bit promotion and ensure overflow bits are dropped).
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x00A9=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetDmgPtr ($CA:62)