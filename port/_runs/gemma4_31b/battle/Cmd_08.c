// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine implements a specific battle command (Cmd_08).
// It selects a magic attack type and result based on a status flag ($2683) 
// and a random value, then triggers the attack and sets message state.
static void Cmd_08_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Check bit 5 of $2683
    if ((ram[0x2683] & 0x20) != 0) {
        ram[0x34CA] = 0x0B;
        ram[0x26D2] = 0x19;
    } else {
        uint8_t r = rand_emu(snes); // jsr Rand (delegated)
        
        if (r >= 0xC0) {             // cmp #$C0 / bcc @e923 (inverted)
            ram[0x34CA] = 0x00;
            ram[0x26D2] = 0x00;
        } else if (r >= 0x80) {      // cmp #$80 / bcc @e930 (inverted)
            ram[0x34CA] = 0x0E;
            ram[0x26D2] = 0x02;
        } else if (r >= 0x40) {      // cmp #$40 / bcc @e93d (inverted)
            ram[0x34CA] = 0x0D;
            ram[0x26D2] = 0x03;
        } else {                     // @e93d
            ram[0x34CA] = 0x0C;
            ram[0x26D2] = 0x29;
        }
    }

    do_magic_attack_emu(snes);      // jsr DoMagicAttack (delegated)
    
    ram[0x34C8] = 0x08;              // set command name
    ram[0x34C7] = 0x10;              // show command name
    
    add_msg3_emu(snes);              // jmp AddMsg3 (delegated)
}

// PITFALLS: 3 (CMP/BCC inversion: bcc branches when A < operand, so we enter 
// the "failure" blocks when A >= operand).
// HELPERS: rand_emu(snes), do_magic_attack_emu(snes), add_msg3_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2683=1
//   output_ram:  0x26D2=1, 0x34CA=1, 0x34C7=1, 0x34C8=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::Cmd_08 ($E9:03)