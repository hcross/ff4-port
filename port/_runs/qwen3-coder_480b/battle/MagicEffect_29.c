// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
//   (battle module convention inferred from context and sibling routines)
//
// Scans forward through $29B5 until $FF is found.
// Stores the index in $8A, then activates monster slot 1.
static void MagicEffect_29_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0x38E6]++;                    // inc $38e6
    uint16_t x = 0;                   // clr_ax (A/X = 0, from DP=0)
    for (;;) {
        uint8_t a = ram[0x29B5 + x];  // lda $29b5,x
        if (a == 0xFF) break;         // cmp #$ff / beq @ddaa
        x++;                          // inx
    }
    write16(ram, 0x8A, x);            // stx $8a
    uint8_t a = 1;                    // lda #$01
    activate_monster_emu(snes, a);    // jmp ActivateMonster (delegated)
}

// PITFALLS: 1 (DB=$7E required for correct RAM addressing),
//           6 (A 8-bit, X 16-bit — must match original mode or parity fails)
// HELPERS: activate_monster_emu(snes, a) — delegates ActivateMonster @ $03:DF5F
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x8A=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_29 ($DD:9B)