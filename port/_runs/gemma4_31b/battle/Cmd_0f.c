// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$E6, DP=0
// Note: This routine manages the "Barrier" (Armor) spell command.
// It sets the spell ID, clears a status flag, executes the magic attack logic,
// adds a message, and then sets the "No Effect" text and command name.
static void Cmd_0f_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x26D2] = 0x05;          // lda #$05 / sta $26d2 (Barrier spell ID)
    ram[0x33C4] = 0;            // stz $33c4
    
    do_magic_attack_emu(snes);  // jsr DoMagicAttack
    add_msg3_emu(snes);         // jsr AddMsg3

    ram[0x34CA] = 0x3A;         // lda #$3a / sta $34ca (Text: "No effect")
    ram[0x34C8] = 0x0F;         // lda #$0f / sta $34c8 (Command name ID)
    ram[0x34C7] = 0x10;         // lda #$10 / sta $34c7 (Show command name flag)
}

// PITFALLS: 1 (DB=$E6 assumed for the routine, but writes to ram[] are absolute
// in this harness, which correctly maps to the WRAM layout).
// HELPERS: do_magic_attack_emu(snes), add_msg3_emu(snes)

// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x26D2=1, 0x33C4=1, 0x34CA=1, 0x34C8=1, 0x34C7=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE6
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::Cmd_0f ($E6:99)