// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: cpu->a = value to clamp, C flag set if value should be clamped to 0xFF
// Output: cpu->a = clamped value (0x00 to 0xFF)
static void Max255_c(Snes *snes) {
    Cpu *cpu = snes->cpu;

    // bcc @9e2b → branch if C clear, skip clamping
    if (cpu->c) {
        // C set: clamp A to 0xFF
        cpu->a = 0xFF;
    }
}

// PITFALLS: 2 (C flag must be set by caller to match `bcc` behavior)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: c=from_caller, z=auto, n=auto
REVERSED_FUNCTION: battle::Max255 ($9E:27)