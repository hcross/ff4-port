// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// All inputs/outputs in WRAM:
//   in : ram[$289E] (AI condition target), ram[$DD] (unknown, likely a flag or index)
//   out: ram[$DE] incremented if conditions are met
//
// Logic:
//   - Calls GetAICondTarget (delegated)
//   - Based on ram[$289E] and ram[$DD], checks various conditions
//   - If a specific condition on $29EB,X (looping over 5 words) is met,
//     ram[$DE] is incremented
static void AICond_03_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    get_ai_cond_target_emu(snes);  // jsr GetAICondTarget

    uint8_t a = ram[0x289E];       // lda $289e
    if (a != 0) goto bdc6;         // bne @bdc6
    a = ram[0xDD];                 // lda $dd
    if (a != 0) goto bded;         // bne @bded
    goto bdef;                     // beq @bdef

bdc6:
    a--;                           // dec
    if (a != 0) goto bdd2;         // bne @bdd2
    a = ram[0xDD];                 // lda $dd
    if (a == 0) goto bded;         // beq @bded
    goto bdef;                     // bne @bdef

bdd2:
    a = ram[0xDD];                 // lda $dd
    if (a == 0) goto bdef;         // beq @bdef

    // clr_ax → tdc / tax (A = D = 0, X = A = 0)
    uint16_t x = 0;                // clr_ax (X 16-bit)
    ram[0xA9] = 0;                 // stx $a9 (only low byte matters)

bdda:
    a = ram[0x29EB + x];           // lda $29eb,x
    if (a == 0) goto bde1;         // beq @bde1
    ram[0xA9]++;                   // inc $a9

bde1:
    x += 2;                        // inx2
    if (x != 10) goto bdda;        // cpx #10 / bne @bdda

    a = ram[0xA9];                 // lda $a9
    a--;                           // dec
    if (a == 0) goto bdef;         // bne @bdef (inverted)

bded:
    ram[0xDE]++;                   // inc $de

bdef:
    return;                        // rts
}

// PITFALLS: 1 (DB=$7E), 8 (mode A 8-bit, X 16-bit inherited from caller)
// HELPERS: get_ai_cond_target_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x289E=1, 0xDD=1
//   output_ram:  0xDE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_03 ($BD:B8)