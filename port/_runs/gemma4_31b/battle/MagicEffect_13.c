// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
//
// Logic:
//   Check bit 0 of ram[0x38E5].
//   If 0: Increment ram[0x38F3], increment local A, then store result in 0x38D3 and 0x35A3.
//   If 1: Clear ram[0x3550], call AddMsg3, store 0x22 in ram[0x34CA].
static void MagicEffect_13_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    uint8_t val = ram[0x38E5];
    if ((val & 0x01) == 0) { // and #$01 / bne @d984 (not taken)
        ram[0x38F3]++;       // inc $38f3
        uint8_t a = ram[0x38F3] + 1; // inc (A is 8-bit)
        ram[0x38D3] = a;     // sta $38d3
        ram[0x35A3] = a;     // sta $35a3
    } else {                // bne @d984 (taken)
        ram[0x3550] = 0;     // stz $3550
        add_msg3_emu(snes);  // jsr AddMsg3 (delegated)
        ram[0x34CA] = 0x22;  // lda #$22 / sta $34ca
    }
}

// PITFALLS: 7 (8-bit arithmetic truncation: inc A wraps at 255), 
//           1 (DB=$7E for WRAM access)
// HELPERS: add_msg3_emu(snes) — delegates AddMsg3 @ $00:85B1
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x38E5=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes
REVERSED_FUNCTION: battle::MagicEffect_13 ($D9:72)