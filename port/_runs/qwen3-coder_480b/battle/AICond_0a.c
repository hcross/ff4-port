// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// All inputs/outputs in WRAM (no register I/O — convention battle):
//   in : ram[$D2] = some index, ram[$34D4..$34D5] = damage data (word per entry)
//   out: ram[$DE] incremented if condition met
static void AICond_0a_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t index = ram[0xD2];
    if (index < 5) return;  // would underflow, but asm does not check
    uint16_t addr = (uint16_t)((index - 5) << 1);  // asl
    uint16_t dmg = read16(ram, 0x34D4 + addr);
    if (dmg == 0) return;  // beq @bf04
    if ((dmg & 0xC000) != 0) return;  // bne @bf04 (MP damage or HP restored)
    ram[0xDE]++;  // inc $de
}

// PITFALLS: 6 (mode A 8-bit assumed from context), 7 (16-bit damage read)
// HELPERS: read16 (for 16-bit access to damage)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0xD2=1, 0x34D4=2
//   output_ram:  0xDE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_0a ($BE:EC)