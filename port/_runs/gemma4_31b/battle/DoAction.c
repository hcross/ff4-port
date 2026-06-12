// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic:
//   1. Reads action type from ram[0x352E].
//   2. Multiplies by 2 (since table is 16-bit addresses, i.e., 2 bytes per entry).
//   3. Look up the jump target in ActionTbl.
//   4. Performs a jump to the resolved address (implemented via emulated function call).
static void DoAction_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // lda $352e / asl
    uint8_t action_type = ram[0x352E];
    uint8_t index = (uint8_t)(action_type << 1); // Pitfall 7: truncate to 8-bit

    // lda ActionTbl,x / sta $80 / lda ActionTbl+1,x / sta $81
    // The table is located at a fixed ROM address (defined as ActionTbl).
    // We fetch the 16-bit address from the table.
    uint16_t target_addr = read16(snes->rom, ACTION_TBL_ADDR + index);

    // lda #^ActionTbl / sta $82
    // This stores the bank of the ActionTbl into $82 for the jml.
    uint8_t target_bank = (uint8_t)(ACTION_TBL_ADDR >> 16);

    // jml [$0080]
    // In the original asm, this is an indirect jump to the address stored at 0x0080.
    // In our C implementation, this translates to executing the function at the resolved 24-bit address.
    uint32_t pc24 = ((uint32_t)target_bank << 16) | target_addr;
    run_emulated_func(snes, pc24);
}

// PITFALLS: 7 (asl A in 8-bit mode truncated to uint8_t)
// HELPERS: read16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x352E=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (indirect jump target varies by ram[0x352E])

REVERSED_FUNCTION: battle::DoAction ($86:87)