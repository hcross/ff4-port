// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine calculates a modified duration by multiplying a base duration
// by a multiplier and then dividing the result by 16 (0x10).
// Logic: ram[0xAB] = (ram[0xA9] * ram[0x3979]) / 16
static void ApplySpeedMod_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Load base duration (8-bit) and multiplier (16-bit)
    uint16_t base_duration = ram[0xA9];         // ldx $a9
    uint16_t multiplier = read16(ram, 0x3979);  // ldx $3979

    // Mult16: (base * mult)
    write16(ram, 0x393D, base_duration);       // stx $393d
    write16(ram, 0x393F, multiplier);          // stx $393f
    mult16_emu(snes);                          // jsr Mult16

    // Result of Mult16 is at 0x3941 (low 16 bits)
    uint16_t product = read16(ram, 0x3941);
    write16(ram, 0x3945, product);             // stx $3945

    // Div16: (product / 0x10)
    write16(ram, 0x3947, 0x0010);              // ldx #$0010 / stx $3947
    div16_emu(snes);                           // jsr Div16

    // Result of Div16 is at 0x3949
    uint16_t result = read16(ram, 0x3949);
    ram[0xAB] = (uint8_t)result;                // stx $ab (8-bit write)
}

// PITFALLS: 1 (DB=0x7E), 8 (X is 16-bit by battle convention)
// HELPERS: mult16_emu(snes), div16_emu(snes), read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00A9=1, 0x3979=2
//   output_ram:  0x00AB=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::ApplySpeedMod ($9F:D8)