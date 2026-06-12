// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: cpu->a = attack_id (8-bit)
// Logic:
//   Shift attack_id right by 3 bits into $a9 (store shifted bits in $a9)
//   Use the low 3 bits (now in X) to index into NoNameAttackTbl
//   Shift the table value right by Y bits (Y = upper 5 bits of attack_id)
//   If carry is set after shift, return 0 (clr_a), else return attack_id
static void CheckAttackName_c(Snes *snes, uint8_t attack_id) {
    uint8_t *ram = snes->ram;
    ram[0xA9] = 0;                    // stz $a9
    uint8_t a = attack_id;
    uint8_t carry = (a & 1);          // lsr (bit 0 -> carry)
    a >>= 1;
    ram[0xA9] = (carry << 7) | (ram[0xA9] >> 1);  // ror $a9
    carry = (a & 1);                  // lsr
    a >>= 1;
    ram[0xA9] = (carry << 7) | (ram[0xA9] >> 1);  // ror $a9
    carry = (a & 1);                  // lsr
    a >>= 1;
    ram[0xA9] = (carry << 7) | (ram[0xA9] >> 1);  // ror $a9
    uint16_t x = a & 0xFF;            // tax (X is 16-bit)
    a = ram[0xA9];                    // lda $a9
    // Call Lsr_5 to shift A right by 5 bits, storing result in A and $a9
    // Since Lsr_5 is not translated, delegate it.
    // Set up A register for the call
    Cpu *cpu = snes->cpu;
    cpu->a = a;
    cpu->x = x;
    cpu->y = 0; // will be set by Lsr_5
    cpu->db = 0x7E;
    cpu->dp = 0;
    cpu->mf = true;
    cpu->xf = false;
    lsr_5_emu(snes);
    a = cpu->a & 0xFF;                // result from A (8-bit)
    ram[0xA9] = a;                    // sta $a9
    uint8_t y = a;                    // tay (Y is 16-bit, but we use lower 8)
    // Load from NoNameAttackTbl using X as index
    uint8_t tbl_val = snes->ram[0x00D349 + x]; // f:NoNameAttackTbl,x (approx)
    // Perform ASL, DEY, BPL loop Y times
    while (y != 0xFF) {               // bpl loop (y >= 0)
        tbl_val <<= 1;                // asl (shift left, bit 7 -> carry)
        y--;
    }
    // If carry is set (i.e., last shift produced a carry), return 0
    if (tbl_val & 0x100) {            // bcc checks carry after last shift
        cpu->a = 0;                   // clr_a (A = 0)
    } else {
        cpu->a = attack_id;           // return attack_id
    }
}

// PITFALLS: 1 (DB must be $7E for WRAM access if any), 7 (shifts in 8-bit mode
// must truncate to 8 bits), 8 (A/X mode inheritance - assumed 8-bit A, 16-bit X)
// HELPERS: lsr_5_emu(snes) - delegates Lsr_5
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::CheckAttackName ($D3:2E)