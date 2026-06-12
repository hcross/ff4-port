// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// Entry: X = object offset, Y = character index (both 16-bit)
// Logic:
//   Loop through characters (Y from 0 to 4):
//     if (char[Y] not present) continue
//     if (obj[X].status & 0xC0 != 0) continue  // dead
//     if (obj[X].status2 & 0x82 != 0) continue // magnetized or jumping
//     if (obj[X].status3 & 0x80 == 0) return  // not hiding
//     NextObj()
//   If looped 5 times, increment $a9 (failure flag)
static void FindValidChar_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // clr_axy → A = X = Y = 0 (D register is 0 in battle)
    cpu->a = 0;
    cpu->x = 0;
    cpu->y = 0;
    ram[0xA9] = 0; // sty $a9 (Y=0)

    for (;;) {
        uint8_t present = ram[0x3540 + cpu->y]; // lda $3540,y
        if (present != 0) goto next;            // bne @c425
        uint8_t status = ram[0x2003 + cpu->x];  // lda $2003,x
        if ((status & 0xC0) != 0) goto next;    // and #$c0 / bne @c425
        uint8_t status2 = ram[0x2005 + cpu->x]; // lda $2005,x
        if ((status2 & 0x82) != 0) goto next;   // and #$82 / bne @c425
        uint8_t status3 = ram[0x2006 + cpu->x]; // lda $2006,x
        if ((status3 & 0x80) == 0) goto done;   // bpl @c430 (not hiding)
next:
        NextObj_emu(snes);                      // jsr NextObj
        cpu->y++;                               // iny
        if (cpu->y == 5) {                      // cpy #5 / bne @c40d
            ram[0xA9]++;                        // inc $a9
            break;
        }
    }
done:
    return;
}

// PITFALLS: 8 (mode A/X inherited as 8-bit/16-bit from caller)
// HELPERS: NextObj_emu(snes) — delegates NextObj @ $03:85BC
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=16
//   inputs_ram:  0x3540=1, 0x2003=1, 0x2005=1, 0x2006=1
//   output_ram:  0x00A9=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::FindValidChar ($C4:08)