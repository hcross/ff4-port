// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$9E, DP=0
// Logic: Clamps the value in the accumulator to a maximum of 99.
// If A > 99, A is set to 99.
static uint8_t Max99_c(Snes *snes) {
    uint8_t a = (uint8_t)snes->cpu->a;

    if (a > 99) {               // cmp #99 / bcc @9e26 (bcs is taken if a >= 99, but here bcc is the skip)
        a = 99;                 // lda #99
    }

    return a;
}

// PITFALLS: 3 (CMP/BCC: bcc branches when A < 99, so we enter the clamp body when A >= 99. 
// Note: in 65816, CMP #99 then BCC @9e26 skips the assignment if A < 99. 
// Since 99 is the limit, and the ASM does 'lda #99' if the branch is NOT taken, 
// the result is A = (A > 99) ? 99 : A).
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x9E
//   entry_flags: z=auto, n=auto
//   custom_spike: yes (returns value via register A)

REVERSED_FUNCTION: battle::Max99 ($9E:20)