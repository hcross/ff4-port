// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This function counts alive party members (not dead or stone) excluding $3540,y == 0
static void CountCharTargets_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0xA9] = 0;                    // stx $a9 (X=0 from clr_ax)
    for (uint8_t y = 0; y < 5; y++) {
        if (ram[0x3540 + y] != 0) {   // bne @ad38 (skip if not present)
            uint8_t status = ram[0x2003 + (y << 1)]; // X is 0, so x=y=0,1,2,3,4 -> obj offset 0,2,4,6,8
            status &= 0xC0;           // and #$c0
            if (status == 0) {        // bne @ad38 (skip if dead or stone)
                ram[0xA9]++;          // inc $a9
            }
        }
        next_obj_emu(snes);           // jsr NextObj
    }
    ram[0x38DB] = ram[0xA9];          // sta $38db
}

// PITFALLS: 1 (DB=$7E required for WRAM access), 8 (A 8-bit, X 16-bit assumed from context)
// HELPERS: next_obj_emu(snes) — delegates NextObj @ $03:85BC
// CONTRACT:
//   inputs_ram: 0x3540=1, 0x2003=1, 0x2005=1, 0x2007=1, 0x2009=1
//   output_ram: 0x38db=1
//   entry_mode: mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::CountCharTargets ($AD:23)