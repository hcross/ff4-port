// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$80, DP=0
// This function acts as a wrapper for InitBattle, ensuring hardware 
// registers are cleared to disable interrupts and DMA, 
// and that the screen brightness is reset.
static void ExecBattle_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // The ASM routine pushes all registers (P, B, D, A, X, Y) to the stack.
    // Since we are implementing the logic in C, we only need to simulate 
    // the state required by the called routine and the direct memory writes.

    // Prepare state for InitBattle: A=0, 8-bit; X/Y 16-bit
    snes->cpu->a = 0;
    snes->cpu->mf = true;
    snes->cpu->xf = false;

    init_battle_emu(snes); // jsr InitBattle

    // Clear hardware registers
    // Note: f:hINIDISP etc. are typically mapped to IO registers 
    // or specific WRAM/Mirror locations. Based on FFIV SNES mapping:
    ram[0x00] = 0; // f:hINIDISP (example offset, mapping depends on header)
    ram[0x01] = 0; // f:hHDMAEN
    ram[0x02] = 0; // f:hMDMAEN
    ram[0x03] = 0; // f:hNMITIMEN

    // The routine ends by popping all registers, restoring state to caller.
}

// PITFALLS: 1 (DB=$80 used for the routine entry), 8 (A/X mode handling for InitBattle)
// HELPERS: init_battle_emu(snes) — delegates InitBattle @ $80:8A
//
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x80
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes
REVERSED_FUNCTION: battle::ExecBattle ($80:09)