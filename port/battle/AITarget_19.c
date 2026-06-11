// This function sets A=0 and jumps to TargetMonsterTypeAll.
// It acts as a wrapper to target all monster types with a multiplier of 0.
static void AITarget_19_c(Snes *snes) {
    snes->cpu->a = 0;
    TargetMonsterTypeAll_emu(snes);
}

// PITFALLS: 1 (DB must be $7E for battle module), 2 (Z/N flags for A=0)
// HELPERS: TargetMonsterTypeAll_emu(snes)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=true, n=false
REVERSED_FUNCTION: battle::AITarget_19 ($B9:38)