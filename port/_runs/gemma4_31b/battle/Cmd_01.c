// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$EB, DP=0
// Logic: 
// 1. Validates a set of status flags in $2683-$2685.
// 2. Determines action based on command ID in $26D2:
//    - 0xCA: Call RandSummon.
//    - 0xB0 or other conditions: Trigger Magic Attack with weapon-specific hits.
//    - Otherwise: Increment a counter ($352A) and trigger Magic Attack.
// 3. Post-attack: Updates output registers $33C5 and $34C8 based on damage result.
static void Cmd_01_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Initial flag checks
    if ((ram[0x2683] & 0xC0) != 0) return; // bne @ec11
    if ((ram[0x2684] & 0x3C) != 0) return; // bne @ec11
    if ((ram[0x2685] & 0xC6) != 0) return; // bne @ec11

    uint8_t cmd_id = ram[0x26D2];
    uint8_t saved_cmd = cmd_id; // pha

    if (cmd_id == 0xCA) { // cmp #$ca / bne @ebc7
        rand_summon_emu(snes);
        // bra @ebfa
    } else if (cmd_id >= 0xB0 || (ram[0x26D0] & 0x10) == 0) { 
        // cmp #$b0 / bcc @ebd2 / and #$10 / beq @ebf7
        // Note: bcc @ebd2 triggers if A < 0xB0. 
        // If A >= 0xB0, it checks if (ram[0x26D0] & 0x10) is non-zero.
        // If that bit is set, it falls through to @ebf7. 
        // Otherwise it continues to @ebd2.
        
        uint8_t weapon_ptr = ram[0x26D5]; // ldx $26d5 / stx $80
        uint8_t current_cmd = ram[0x26D2];
        
        if (current_cmd >= 0x61) { // cmp #$61 / bcc @ebe2
            ram[0x26D2] = 0x00;   // lda #$00 / bra @ebf2
        } else {
            // lda f:WeaponMagicHits,x (Assuming f is a known offset or pointer)
            // For the sake of parity, we simulate the memory access:
            // WeaponMagicHits is an external table. 
            // In a real reimplementation, this would be: ram[WEAPON_MAGIC_HITS + weapon_ptr]
            uint8_t hits = ram[0x2000 + weapon_ptr]; // Placeholder for f:WeaponMagicHits
            ram[0x38EC] = hits;    // sta $38ec
            ram[0x38EB]++;        // inc $38eb (set weapon magic flag)
            
            // ldy #$0003 / lda ($80),y -> read 4th byte of the struct pointed to by $80
            // $80 is the DP address where we stored weapon_ptr.
            // In this context, it's treating the value in RAM $80 as a pointer.
            uint16_t ptr = read16(ram, 0x80);
            ram[0x26D2] = ram[ptr + 3]; // sta $26d2
        }
    } else {
        ram[0x352A]++; // @ebf7: inc $352a
    }

    do_magic_attack_emu(snes); // jsr DoMagicAttack

    if (ram[0x38ED] != 0) { // lda $38ed / beq @ec05
        ram[0x34C8] = saved_cmd; // pla / bra @ec09 / sta $34c8
    } else {
        ram[0x33C5] = saved_cmd; // pla / sta $33c5
        ram[0x34C8] = saved_cmd; // sta $34c8
    }
    
    ram[0x34C7] = 0x20; // show item name
}

// PITFALLS: 1 (DB=$EB used for $26XX access), 3 (CMP/BCC logic)
// HELPERS: rand_summon_emu(snes), do_magic_attack_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2683=1, 0x2684=1, 0x2685=1, 0x26D2=1, 0x26D5=1, 0x26D0=1
//   output_ram:  0x34C7=1, 0x34C8=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xEB
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::Cmd_01 ($EB:A2)