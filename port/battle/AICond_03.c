// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This function checks an AI condition based on target and status.
// It modifies $de (condition result flag) in WRAM.
static void AICond_03_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    get_ai_cond_target_emu(snes);  // jsr GetAICondTarget

    uint8_t a = ram[0x289E];       // lda $289e
    if (a != 0) goto label_bdc6;

    a = ram[0xDD];                 // lda $dd
    if (a != 0) goto label_bded;
    goto label_bdef;

label_bdc6:
    a = ram[0x289E];
    a--;                           // dec
    if (a != 0) goto label_bdd2;

    a = ram[0xDD];
    if (a == 0) goto label_bded;
    goto label_bdef;

label_bdd2:
    a = ram[0xDD];
    if (a == 0) goto label_bdef;

    // clr_ax → tdc / tax (A = X = DP = 0, but DP=0 so A/X = 0)
    uint16_t x = 0;
    ram[0xA9] = 0;                 // stx $a9 (low byte of X)

label_bdda:
    a = ram[0x29EB + x];           // lda $29eb,x
    if (a == 0) goto label_bde1;
    ram[0xA9]++;                   // inc $a9

label_bde1:
    x += 2;                        // inx2
    if (x != 10) goto label_bdda;  // cpx #10 / bne

    a = ram[0xA9];
    a--;                           // dec
    if (a != 0) goto label_bdef;   // bne @bdef

label_bded:
    ram[0xDE]++;                   // inc $de

label_bdef:
    return;
}

// PITFALLS: 1 (DB must be $7E for WRAM access), 8 (mode A 8-bit assumed),
//           9 (clr_ax is tdc/tax, not lda #0/ldx #0)
// HELPERS: get_ai_cond_target_emu(snes) — delegates GetAICondTarget @ $00:BF0F
// CONTRACT:
//   inputs_ram: 0x289e=1, 0xdd=1
//   output_ram: 0xde=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_03 ($BD:B8)