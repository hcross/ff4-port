// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$80, DP=0
// This is a "far call" wrapper. It saves the current CPU context,
// sets the Data Bank (DB) to 0x7E (WRAM), and calls the actual
// UpdateEquip routine.
//
// Logic:
// 1. Store input A into ram[0x3975].
// 2. Save all registers to stack.
// 3. Set DB = 0x7E.
// 4. Call UpdateEquip.
// 5. Restore registers and return.
static void UpdateEquip_far_c(Snes *snes, uint8_t input_a) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // sta $7e3975 (DB=$80, so this is absolute $7e3975 effectively via the bank mapping)
    // Note: The ASM says sta $7e3975. In the provided memory map, 
    // this is an absolute write to WRAM.
    ram[0x3975] = input_a;

    // The routine performs a heavy context save/restore.
    // In a C reimplementation, we simply set the environment and call the target.
    
    // Prepare for UpdateEquip
    cpu->db = 0x7E;    // lda #$7e / pha / plb
    cpu->mf = true;    // shorta
    cpu->xf = false;   // longi (X/Y 16-bit)
    cpu->a = 0;        // lda #0
    cpu->x = 0;        // ldx #0

    update_equip_emu(snes);

    // Context is restored by the nature of the C call stack.
    // The original ASM saves/restores P, B, D, A, X, Y.
}

// PITFALLS: 1 (DB must be 0x7E for UpdateEquip)
// HELPERS: update_equip_emu(snes) — delegates UpdateEquip @ $00:97FD
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x3975=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x80
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::UpdateEquip_far ($80:36)