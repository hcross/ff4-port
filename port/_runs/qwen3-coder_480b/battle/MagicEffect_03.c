// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This function sets a target's HP to a random value (1-10) if it's higher,
// otherwise removes the target.
static void MagicEffect_03_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    cpu->x = 1;                          // ldx #1
    cpu->a = 9;                          // lda #9
    RandXA_emu(snes);                    // jsr RandXA
    // RandXA returns result in A, but code uses TAX / STX $a9
    cpu->x = cpu->a & 0xFF;              // tax (X 16-bit, but only low byte set)
    ram[0xA9] = cpu->x & 0xFF;           // stx $a9

    cpu->mf = false;                     // longa
    uint16_t target_hp = read16(ram, 0x2707); // lda $2707
    uint16_t rand_hp = (uint16_t)ram[0xA9];   // lda $a9 (now 16-bit)
    if (target_hp < rand_hp) {           // cmp $a9 / bcc (inverted!)
        write16(ram, 0x2707, rand_hp);   // lda $a9 / sta $2707
        cpu->mf = true;                  // shorta
        return;
    } else {
        cpu->mf = true;                  // shorta0
        RemoveTarget_emu(snes);          // jsr RemoveTarget
        return;
    }
}

// PITFALLS: 1 (DB=$7E required for 16-bit absolute access),
//           3 (CMP/BCC inversion: bcc branches when A<C, so we enter body when A>=C),
//           6 (Mode A/X tracking: routine switches from 8-bit A to 16-bit A)
// HELPERS: RandXA_emu(snes), RemoveTarget_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x2707=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_03 ($D4:43)