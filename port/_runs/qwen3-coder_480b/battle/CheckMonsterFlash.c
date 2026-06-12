// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: cpu->a = attack_id (8-bit)
// Logic:
//   1. Shift attack_id right by 3 bits into $a9 (result: $a9 = attack_id >> 3)
//   2. Index NoMonsterFlashTbl with low 3 bits of attack_id (X = attack_id & 7)
//   3. Shift the table byte right by Y ($a9) bits using Lsr_5
//   4. If carry is set after shift, return shifted value; else return 0
static void CheckMonsterFlash_c(Snes *snes, uint8_t attack_id) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    ram[0xA9] = 0;
    uint8_t a = attack_id;

    // Perform 3 right shifts with carry propagation into $a9
    ram[0xA9] = (ram[0xA9] >> 1) | ((a & 1) << 7); a = (uint8_t)(a >> 1); // lsr / ror $a9
    ram[0xA9] = (ram[0xA9] >> 1) | ((a & 1) << 7); a = (uint8_t)(a >> 1); // lsr / ror $a9
    ram[0xA9] = (ram[0xA9] >> 1) | ((a & 1) << 7); a = (uint8_t)(a >> 1); // lsr / ror $a9

    uint16_t x = a & 0x07; // tax (X is 16-bit, so mask to 3 bits)

    uint8_t a9 = ram[0xA9];
    cpu->a = a9;
    cpu->z = (a9 == 0);
    cpu->n = (a9 & 0x80) != 0;
    Lsr_5_emu(snes); // jsr Lsr_5
    a9 = (uint8_t)cpu->a;
    ram[0xA9] = a9;

    uint8_t y = a9;
    uint8_t acc = snes->rom[0x038388 + x]; // f:NoMonsterFlashTbl,x (bank $03, offset 0x8388)

    // asl / dey / bpl loop: shift left Y times or until Y < 0
    while ((int8_t)y >= 0) {
        cpu->c = (acc & 0x80) != 0; // Set carry before shift
        acc = (uint8_t)(acc << 1);
        y--;
    }

    // bcc @d376: branch if carry clear (no carry from last asl)
    if (cpu->c) { // carry set?
        cpu->a = (uint16_t)(acc >> 1); // ror: shift back and set carry
        return;
    }

    cpu->a = 0; // clr_a
}

// PITFALLS: 1 (DB must be $7E for RAM access), 6 (A is 8-bit), 7 (shift truncation),
//           8 (A/X mode inherited as 8/16)
// HELPERS: Lsr_5_emu(snes)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::CheckMonsterFlash ($D3:54)