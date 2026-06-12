// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$80, DP=0
// This is a thin jump wrapper that delegates the actual execution 
// to the extended version of the routine.
static void ExecBtlGfx_c(Snes *snes) {
    exec_btlgfx_ext_emu(snes);
}

// PITFALLS: None (simple delegation)
// HELPERS: exec_btlgfx_ext_emu(snes) — delegates ExecBtlGfx_ext @ $80:0003
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x80
//   entry_flags: z=auto, n=auto

REVERSED_FUNCTION: battle::ExecBtlGfx ($80:85)