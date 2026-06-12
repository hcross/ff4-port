// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Iterates through 5 character slots to count those that are 
// present in battle and neither dead nor petrified.
//
// Logic:
//   For y = 0 to 4:
//     if (char_present[y] != 0) AND (char_status[x] & 0xC0 == 0):
//       count++
//     NextObj() updates the index x for the current character slot y
static void CountCharTargets_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // clr_ax / txy / stx $a9
    // DP=0, so tdc clears A and X. Result: ram[0xA9] = 0
    ram[0xA9] = 0;
    
    for (uint16_t y = 0; y < 5; y++) {
        // lda $3540,y : Check if character is present in battle
        if (ram[0x3540 + y] == 0) {
            goto next_obj_call; // bne @ad38 (skipped if 0)
        }
        
        // lda $2003,x : Check status (dead/stone)
        // Note: x is managed by NextObj. We must use the current emulated x.
        uint8_t status = ram[0x2003 + snes->cpu->x];
        if ((status & 0xC0) != 0) {
            goto next_obj_call; // bne @ad38
        }
        
        ram[0xA9]++; // inc $a9
        
    next_obj_call:
        next_obj_emu(snes); // jsr NextObj
    }
    
    // lda $a9 / sta $38db
    ram[0x38DB] = ram[0xA9];
}

// PITFALLS: 5 (clr_ax is tdc/tax), 8 (inherited mf=true for battle module)
// HELPERS: next_obj_emu(snes) — delegates NextObj @ $85BC
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3540=1, 0x2003=1
//   output_ram: 0x38DB=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto

REVERSED_FUNCTION: battle::CountCharTargets ($AD:23)