// Entry mode: A 16-bit (mf=0), X 16-bit (xf=0), DB=$7E, DP=0
// No inputs or outputs in registers; all state is internal or in WRAM.
// This is a pure wrapper around ExecBtlGfx_ext.
static void ExecBtlGfx_c(Snes *snes) {
    exec_btl_gfx_ext_emu(snes);  // jsl ExecBtlGfx_ext
}

// PITFALLS: 1 (DB must be $7E for ExecBtlGfx_ext, which is in bank $C0)
// HELPERS: exec_btl_gfx_ext_emu(snes) — delegates ExecBtlGfx_ext @ $C0:xxxx
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::ExecBtlGfx ($80:85)