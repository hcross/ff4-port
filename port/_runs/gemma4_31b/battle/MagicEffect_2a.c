// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Update specific magic effect flags and set a timer/state value.
// Logic:
//   1. Backup ram[$2704] to ram[$A9]
//   2. Mask ram[$2704] with 0xBB
//   3. Mask ram[$2706] with 0xC3
//   4. Set ram[$273B] to 0x10
static void MagicEffect_2a_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    uint8_t val2704 = ram[0x2704];
    ram[0xA9] = val2704;               // sta $a9
    ram[0x2704] = val2704 & 0xBB;      // and #$bb / sta $2704

    uint8_t val2706 = ram[0x2706];
    ram[0x2706] = val2706 & 0xC3;      // and #$c3 / sta $2706

    ram[0x273B] = 0x10;                 // lda #$10 / sta $273b
}

// PITFALLS: 1 (DB=$7E required for WRAM access)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2704=1, 0x2706=1
//   output_ram:  0xA9=1, 0x2704=1, 0x2706=1, 0x273B=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_2a ($DD:B1)