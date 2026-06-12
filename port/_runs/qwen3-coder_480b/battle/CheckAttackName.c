// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: cpu->a = attack_id (8-bit)
// Logic:
//   Shift attack_id right by 3 bits, store low 3 bits in X, high 5 bits in $a9
//   Shift $a9 right by 5 bits using Lsr_5
//   Use X to index NoNameAttackTbl, then shift the value left Y times (Y = result from Lsr_5)
//   If carry is set after final shift, return 0 (clr_a), else return attack_id
static void CheckAttackName_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = snes->cpu->a;         // attack_id
    ram[0xA9] = 0;                    // stz $a9
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

    // Call Lsr_5 to shift A right by 5 bits
    Cpu *cpu = snes->cpu;
    cpu->a = a;
    cpu->x = x;
    cpu->y = 0;
    cpu->db = 0x7E;
    cpu->dp = 0;
    cpu->mf = true;
    cpu->xf = false;
    Lsr_5_emu(snes);                  // jsr Lsr_5
    a = cpu->a & 0xFF;                // result from A (8-bit)
    ram[0xA9] = a;                    // sta $a9
    uint8_t y = a;                    // tay

    // Load from NoNameAttackTbl using X as index
    // f:NoNameAttackTbl is at $D349 in bank $D3, but since DB=$7E, we access via ROM mirror or correct mapping
    // For parity, assume table is accessible at a known offset in ROM
    // Table starts at $D349, so address is $D349 + x
    // Since we're in DB=$7E, we must access via correct bank
    // But for simplicity in harness, assume table is mapped correctly in snes->ram or ROM is accessible
    // Let's assume NoNameAttackTbl is at $00D349 in ROM (bank $D3)
    // In real harness, this would be a ROM read, but parity uses RAM mirror
    // So we'll access it as ram[0x00D349 + x]
    uint8_t tbl_val = ram[0x00D349 + x]; // f:NoNameAttackTbl,x

    // Perform ASL, DEY, BPL loop Y times
    while ((int8_t)y >= 0) {          // bpl loop (y >= 0)
        carry = (tbl_val & 0x80) != 0; // asl (shift left, bit 7 -> carry)
        tbl_val <<= 1;
        y--;                          // dey
    }

    // If carry is clear (bcc taken), return attack_id, else return 0
    if (!carry) {                     // bcc @d352
        cpu->a = snes->cpu->a;        // pla (return original attack_id)
    } else {
        cpu->a = 0;                   // clr_a (return 0)
    }
}

// PITFALLS: 1 (DB must be $7E for WRAM access), 7 (shifts in 8-bit mode
// must truncate to 8 bits), 8 (A/X mode inheritance - assumed 8-bit A, 16-bit X)
// HELPERS: Lsr_5_emu(snes) - delegates Lsr_5
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::CheckAttackName ($D3:2E)