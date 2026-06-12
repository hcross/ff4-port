// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Resets a specific battle state byte and triggers the multi-attack logic.
static void Cmd_0e_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xC1] = 0; // stz $c1

    do_multi_attack_emu(snes); // jmp DoMultiAttack
}

// PITFALLS: None (Simple assignment and jump)
// HELPERS: do_multi_attack_emu(snes) — delegates DoMultiAttack @ $E6:B7+5
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x00C1=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto

REVERSED_FUNCTION: battle::Cmd_0e ($E6:B7)