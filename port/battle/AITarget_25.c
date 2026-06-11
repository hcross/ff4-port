// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// All inputs/outputs in WRAM:
//   in : ram[$29CD] counter
//   out: ram[$AF]=$05, ram[$B0]=$0C, ram[$AD]=ram[$D2]
//        then jumps to RandAITarget
static void AITarget_25_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t counter = ram[0x29CD];
    counter--;                          // dec
    if (counter != 0) {                 // bne @bb16
        ram[0xAF] = 0x05;
        ram[0xB0] = 0x0C;
        ram[0xAD] = ram[0xD2];
        randaitarget_emu(snes);         // jmp RandAITarget
        return;
    }
    skipaiturn_emu(snes);               // jmp SkipAITurn
}

// PITFALLS: 1 (DB=$7E required for WRAM access)
// HELPERS: randaitarget_emu(snes), skipaiturn_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x29CD=1, 0xD2=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: auto
REVERSED_FUNCTION: battle::AITarget_25 ($BB:0D)