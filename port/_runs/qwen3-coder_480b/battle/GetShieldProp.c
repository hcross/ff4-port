// Increments shield multiplier and accumulates shield properties
// from indirect table pointed to by ($82),Y into various battle RAM fields.
static void GetShieldProp_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    ram[0x396B]++; // inc $396b

    // Process magic defense % (bits 0-6)
    uint16_t addr = (ram[0x83] << 8) | ram[0x82];
    uint8_t val = ram[addr + cpu->y];
    val &= 0x7F;
    ram[0x3972] = (uint8_t)(ram[0x3972] + val); // adc $3972
    cpu->y++; // iny

    // Process defense
    val = ram[addr + cpu->y];
    ram[0x396F] = (uint8_t)(ram[0x396F] + val); // adc $396f
    cpu->y++; // iny

    // Process defense % (bits 0-6)
    val = ram[addr + cpu->y];
    val &= 0x7F;
    ram[0x396C] = (uint8_t)(ram[0x396C] + val); // adc $396c
    cpu->y++; // iny

    // Process magic defense
    val = ram[addr + cpu->y];
    ram[0x3973] = (uint8_t)(ram[0x3973] + val); // adc $3973
    cpu->y += 2; // iny2

    // Process creature type (OR'd)
    val = ram[addr + cpu->y];
    ram[0x3974] |= val; // ora $3974
    cpu->y++; // iny

    // Process element
    val = ram[addr + cpu->y];
    ram[0x396D] = val; // sta $396d
    cpu->y++; // iny

    // Process status 1 (OR'd)
    val = ram[addr + cpu->y];
    ram[0x3970] |= val; // ora $3970
    cpu->y++; // iny

    // Process status 2 (OR'd)
    val = ram[addr + cpu->y];
    ram[0x3971] |= val; // ora $3971
}

// PITFALLS: 6 (mode A assumed 8-bit), 7 (ADC truncation to 8-bit),
//           9 (indirect addressing via ($82),Y requires full Y register)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  y=16
//   inputs_ram:  0x82=2, 0x396B=1, 0x396C=1, 0x396F=1, 0x3970=1, 0x3971=1, 0x3972=1, 0x3973=1, 0x3974=1
//   output_ram:  0x396B=1, 0x396C=1, 0x396D=1, 0x396F=1, 0x3970=1, 0x3971=1, 0x3972=1, 0x3973=1, 0x3974=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetShieldProp ($9D:AB)