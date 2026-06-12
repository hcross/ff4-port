// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: no register inputs, all state in WRAM
// Logic:
//   if ($28a3 >= 0) || ($2740 & 0x8a == 0) → SetMagicStatus2
//   else → _d5a2
static void SetMagicStatus_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a3 = ram[0x28A3];
    if ((a3 & 0x80) == 0) {              // bpl → N=0 when bit 7 clear
        set_magic_status2_emu(snes);
        return;
    }
    uint8_t flags = ram[0x2740] & 0x8A;  // and #$8a
    if (flags == 0) {                    // beq → Z=1 when result zero
        set_magic_status2_emu(snes);
        return;
    }
    _d5a2_emu(snes);                     // jmp _d5a2
}

// PITFALLS: 1 (DB=$7E required for WRAM access), 2 (flags Z/N set by caller)
// HELPERS: set_magic_status2_emu(snes), _d5a2_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x28a3=1, 0x2740=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::SetMagicStatus ($D5:05)