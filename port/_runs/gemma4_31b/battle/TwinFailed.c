// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine sets a failure state for a "Twin" encounter and 
// jumps to AddMsg2 to handle the message queue.
static void TwinFailed_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x357B] = 0xFF; // lda #$ff / sta $357b
    ram[0x34CA] = 0x11; // lda #$11 / sta $34ca

    add_msg2_emu(snes);  // jmp AddMsg2 (delegated)
}

// PITFALLS: None applicable. Simple immediate loads and stores.
// HELPERS: add_msg2_emu(snes) — delegates AddMsg2 @ 0x0085A6
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x357B=1, 0x34CA=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto

REVERSED_FUNCTION: battle::TwinFailed ($E4:D9)