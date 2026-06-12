// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// This routine reads shield properties from a table pointed to by ($82, Y)
// and accumulates them into specific WRAM addresses.
//
// Inputs:
//   cpu->y: offset into the shield property table
//   ram[0x0082]: pointer to the property table (16-bit LE)
//   ram[0x396B, 0x396C, 0x396F, 0x3970, 0x3971, 0x3972, 0x3973, 0x3974]: current accumulated values
//
// Logic:
//   - Increments a shield multiplier counter.
//   - Reads 8 properties sequentially from the table.
//   - Some are summed (adc), some are bitwise-ORed (ora), and some are direct copies.
static void GetShieldProp_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t y = snes->cpu->y;
    uint16_t ptr = read16(ram, 0x0082);

    ram[0x396B]++; // inc $396b

    // 1. Mag Def %: (val & 0x7F) + ram[0x3972]
    uint8_t val0 = ram[ptr + y];
    ram[0x3972] = (uint8_t)((val0 & 0x7F) + ram[0x3972]); // Pitfall 7: truncate to 8-bit
    y++;

    // 2. Defense: val + ram[0x396F]
    uint8_t val1 = ram[ptr + y];
    ram[0x396F] = (uint8_t)(val1 + ram[0x396F]); // Pitfall 7
    y++;

    // 3. Defense %: (val & 0x7F) + ram[0x396C]
    uint8_t val2 = ram[ptr + y];
    ram[0x396C] = (uint8_t)((val2 & 0x7F) + ram[0x396C]); // Pitfall 7
    y++;

    // 4. Mag Def: val + ram[0x3973]
    uint8_t val3 = ram[ptr + y];
    ram[0x3973] = (uint8_t)(val3 + ram[0x3973]); // Pitfall 7
    y += 2; // iny2

    // 5. Creature Type: val | ram[0x3974]
    uint8_t val4 = ram[ptr + y];
    ram[0x3974] = val4 | ram[0x3974];
    y += 3; // iny3

    // 6. Element: direct copy to ram[0x396D]
    uint8_t val5 = ram[ptr + y];
    ram[0x396D] = val5;
    y++;

    // 7. Status 1: val | ram[0x3970]
    uint8_t val6 = ram[ptr + y];
    ram[0x3970] = val6 | ram[0x3970];
    y++;

    // 8. Status 2: val | ram[0x3971]
    uint8_t val7 = ram[ptr + y];
    ram[0x3971] = val7 | ram[0x3971];

    snes->cpu->y = y; // Update Y for caller
}

// PITFALLS: 7 (8-bit arithmetic truncation for ADC), 1 (DB=$7E implicitly handled via absolute WRAM indexing)
// HELPERS: read16 (used to resolve the indirect pointer at $82)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=16
//   inputs_ram:  0x0082=2, 0x396B=1, 0x396C=1, 0x396D=1, 0x396F=1, 0x3970=1, 0x3971=1, 0x3972=1, 0x3973=1, 0x3974=1
//   output_ram:  0x396B=1 (increments, but other RAM values are modified)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   CUSTOM_SPIKE: yes (multiple outputs, depends on pointer at $82)
REVERSED_FUNCTION: battle::GetShieldProp ($9D:AB)