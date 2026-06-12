// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: ram[$352E] = action type index (0-255)
// Logic:
//   Reads a jump table at f:ActionTbl (ROM mirror $C00000+)
//   Computes jump address as ActionTbl[action_type * 2]
//   Jumps to that address (in ROM, bank $C0+)
static void DoAction_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t action_type = ram[0x352E];           // lda $352e
    uint16_t x = (uint16_t)(action_type << 1);   // asl / tax (X 16-bit)
    // ROM mirror assumed at $C00000+ (typical for LakeSnes)
    uint32_t action_tbl_addr = 0xC00000 + x;
    uint16_t jump_addr = (uint16_t)(snes->ram[action_tbl_addr] |
                                   (snes->ram[action_tbl_addr + 1] << 8));
    // Store jump vector in $80-$82 (standard RAM vector)
    ram[0x80] = jump_addr & 0xFF;
    ram[0x81] = (jump_addr >> 8) & 0xFF;
    ram[0x82] = 0xC0;  // bank byte for ActionTbl
    // Jump to [$0080] = [$80:$81] in bank $82
    // Emulate the jump by setting PC and returning to emulator loop
    snes->cpu->pc = jump_addr;
    snes->cpu->k = 0xC0;
}

// PITFALLS: 1 (DB=$7E assumed for WRAM access), 6 (A 8-bit mode assumed
// from context — no explicit mode set, but battle uses shorta by default)
// HELPERS: none (no jsr, no sub-routine calls)
// CONTRACT:
//   inputs_ram:  0x352E=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::DoAction ($86:87)