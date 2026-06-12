// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$E8, DP=0
// This routine saves two specific state bytes, clears/decrements them to 
// trigger a specific combat action (DoFightCmd), and then restores the 
// original values.
//
// Logic:
//   1. Push ram[0x269C] and ram[0x2729] to stack.
//   2. Set ram[0x2729] to 0.
//   3. Decrement ram[0x269C] (resulting in 0xFF if it was 0).
//   4. Execute DoFightCmd.
//   5. Restore original ram[0x2729] and ram[0x269C].
static void Cmd_0c_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Temporary storage for values pushed to stack
    uint8_t old_269c = ram[0x269C];
    uint8_t old_2729 = ram[0x2729];

    // clr_a (tdc) results in A=0 assuming DP=0
    ram[0x2729] = 0; 
    
    // dec $269c: Pitfall 7 (8-bit truncation/wrap)
    ram[0x269C] = (uint8_t)(ram[0x269C] - 1);

    do_fight_cmd_emu(snes); // jsr DoFightCmd

    // Restore values from stack
    ram[0x2729] = old_2729;
    ram[0x269C] = old_269c;
}

// PITFALLS: 7 (Arithmetic truncation: dec $269c must wrap at 0)
// HELPERS: do_fight_cmd_emu(snes) — delegates DoFightCmd @ $E8:C489
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x269C=1, 0x2729=1
//   output_ram:  none (state is restored, output is side-effect of DoFightCmd)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE8
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::Cmd_0c ($E8:39)