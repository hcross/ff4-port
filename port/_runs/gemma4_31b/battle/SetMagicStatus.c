// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   Checks magic status/flag at $28A3. If negative (bit 7 set), 
//   checks if specific bits are set in $2740.
//   Depending on these conditions, it either branches to 
//   SetMagicStatus2 or jumps to the handler at _d5a2.
static void SetMagicStatus_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    uint8_t val28a3 = ram[0x28A3];
    if ((int8_t)val28a3 >= 0) {             // bpl SetMagicStatus2
        set_magic_status2_emu(snes);
        return;
    }

    uint8_t val2740 = ram[0x2740];
    if ((val2740 & 0x8A) == 0) {            // and #$8a / beq SetMagicStatus2
        set_magic_status2_emu(snes);
        return;
    }

    // jump to _d5a2
    _d5a2_emu(snes);
}

// PITFALLS: None specific, standard 8-bit logic.
// HELPERS: set_magic_status2_emu(snes), _d5a2_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x28A3=1, 0x2740=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes
REVERSED_FUNCTION: battle::SetMagicStatus ($D5:05)