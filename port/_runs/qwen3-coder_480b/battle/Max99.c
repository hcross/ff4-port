// Constrains A to at most 99. If A >= 99, sets A = 99.
// Entry: A = value to clamp (8-bit), Z/N = flags set accordingly
// Exit:  A = min(A, 99)
static void Max99_c(Snes *snes) {
    Cpu *cpu = snes->cpu;
    uint8_t a = cpu->a & 0xFF;  // A is 8-bit (mf=1 assumed)
    if (a >= 99) {              // cmp #99 / bcc @9e26
        cpu->a = 99;            // lda #99
    }
    // else fall through to rts
}

// PITFALLS: 6 (A is 8-bit: cmp #99 implies 8-bit mode)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::Max99 ($9E:0020)