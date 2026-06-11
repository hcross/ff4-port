// AI monster selects a target (character or monster-specific handler)
// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// 
// Inputs:
//   cpu->x = value to preserve in $88
//   ram[$361C] = AI script usage counter index
//   ram[$A6] = attacker object index (16-bit)
//   ram[$A9] = target specifier:
//     $00-$15: character ID to search for
//     $16+   : monster-specific target handler via AITargetTbl
//
// Outputs (character path):
//   ram[$2054 + attacker] = target bitmask (bit 7-Y set, where Y is char slot)
//
// Outputs (monster path):
//   Tail-jumps to handler via AITargetTbl; no return
static void GetAITarget_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;
    
    // stx $88 — preserve X
    write16(ram, 0x88, cpu->x);
    
    // lda $361c / tax / inc $3879,x — increment AI script usage counter
    uint8_t counter_idx = ram[0x361C];
    ram[0x3879 + (uint16_t)counter_idx]++;  // tax zero-extends to 16-bit
    
    // ldx $a6 — load attacker index
    uint16_t attacker_idx = read16(ram, 0xA6);
    
    // stz $2053,x / stz $2054,x — clear target field
    ram[0x2053 + attacker_idx] = 0;
    ram[0x2054 + attacker_idx] = 0;
    
    // lda $a9 / cmp #$16 / bcc @b8d0
    uint8_t target = ram[0xA9];
    
    if (target >= 0x16) {
        // Monster target path: lookup handler in AITargetTbl and tail-jump
        // sec / sbc #$16 / asl / tax
        uint8_t offset = (uint8_t)((target - 0x16) << 1);  // Pitfall 7: truncate
        uint16_t tbl_idx = (uint16_t)offset;
        
        // lda f:AITargetTbl,x / sta $a9
        // lda f:AITargetTbl+1,x / sta $aa
        // lda #$03 / sta $ab
        // We need AITargetTbl address — delegate entire jump table dispatch
        cpu->db = 0x7E;
        cpu->dp = 0;
        cpu->mf = true;   // A 8-bit
        cpu->xf = false;  // X 16-bit
        cpu->x = tbl_idx;
        run_emulated_func(snes, 0x03B8BDu);  // "lda f:AITargetTbl,x"
        return;  // jml [$00a9] never returns
    }
    
    // Character target path: search slots 0-4 for matching character
    // @b8d0: clr_axy — A=X=Y=0 (DP=0)
    uint16_t x = 0;
    uint16_t y = 0;
    
    for (;;) {
        // @b8d3: lda $3540,y / bne @b8f4
        if (ram[0x3540 + y] != 0) {
            goto skip_slot;  // slot present (unavailable)
        }
        
        // lda $2000,x / and #$1f / cmp $a9 / bne @b8f4
        if ((ram[0x2000 + x] & 0x1F) != target) {
            goto skip_slot;  // wrong character ID
        }
        
        // lda $2003,x / and #$c0 / bne @b8fd
        if ((ram[0x2003 + x] & 0xC0) != 0) {
            // Dead or stone — @b8fd: jmp SkipAITurn
            skipaiturn_emu(snes);
            return;
        }
        
        // lda $2005,x / and #$82 / bne @b8fd
        if ((ram[0x2005 + x] & 0x82) != 0) {
            // Magnetized or jumping — @b8fd: jmp SkipAITurn
            skipaiturn_emu(snes);
            return;
        }
        
        // lda $2006,x / bpl @b900
        if ((ram[0x2006 + x] & 0x80) == 0) {
            // Not hiding — target found!
            break;
        }
        
skip_slot:
        // @b8f4: jsr NextObj
        nextobj_emu(snes);
        x = cpu->x;  // NextObj advances X (object pointer)
        
        // iny / cpy #5 / bne @b8d3
        y++;
        if (y >= 5) {
            // Loop exhausted, no valid target — @b8fd: jmp SkipAITurn
            skipaiturn_emu(snes);
            return;
        }
    }
    
    // @b900: Target found at slot Y
    // ldx $a6
    uint16_t attacker = read16(ram, 0xA6);
    
    // sec / @b903: ror $2054,x / dey / bpl @b903
    // Builds bitmask: 0x80 >> Y (so Y=0 → 0x80, Y=1 → 0x40, etc.)
    uint8_t mask = 0;
    bool carry = true;
    do {
        uint8_t bit0 = mask & 0x01;
        mask = (mask >> 1) | (carry ? 0x80 : 0);
        carry = (bit0 != 0);
        y--;  // dey (16-bit decrement)
    } while ((int16_t)y >= 0);  // bpl: branch if positive
    
    ram[0x2054 + attacker] = mask;
    // rts
}

// PITFALLS: 7 (arithmetic truncation in offset<<1), 1 (DB=$7E for emulated calls)
// HELPERS: nextobj_emu, skipaiturn_emu, run_emulated_func (for AITargetTbl dispatch)
// CONTRACT:
//   inputs_reg:  x=16
//   inputs_ram:  0x361C=1, 0xA6=2, 0xA9=1
//                0x3540+[0..4]=1, 0x2000+x=1, 0x2003+x=1, 0x2005+x=1, 0x2006+x=1
//   output_ram:  0x2054+0xA6=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: auto
// CUSTOM_SPIKE: yes
REVERSED_FUNCTION: battle::GetAITarget ($03:B8A1)