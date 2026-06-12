// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// Logic: Iterates through a list of target indices. For each target, it checks
// if the entity is alive and not petrified (via $2003 + offset). If valid,
// it marks it in the bitmask at $AD by calling ClearBit.
static void ClearInvalidTargets_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xAA] = 0; // stz $aa (counter)
    
    // Y is expected to be the base address of the target list at entry
    uint16_t y = snes->cpu->y; 
    uint8_t counter = 0;

    while (counter != ram[0xAB]) { // cmp $ab / bne @a74f
        // asl $a9: check if bit 7 of the target indicator is set
        uint8_t indicator = ram[0xA9];
        uint8_t carry = (indicator & 0x80) != 0;
        ram[0xA9] = (uint8_t)(indicator << 1); // Pitfall 7: wrap to 8-bit

        if (!carry) { // bcc @a764
            // check status: lda $2003,y / and #$c0
            // $2003 is a hardware register or special memory offset in battle context
            uint8_t status = ram[0x2003 + y]; 
            if ((status & 0xC0) == 0) { // beq @a764
                // Clear the bit for this target
                snes->cpu->a = ram[0xAA]; // lda $aa
                snes->cpu->x = snes->cpu->a; // tax
                snes->cpu->a = ram[0xAD]; // lda $ad
                
                clear_bit_emu(snes); // jsr ClearBit (delegated)
                
                ram[0xAD] = snes->cpu->a; // sta $ad
            }
        }

        // Update Y: longa / tya / clc / adc #$0080 / tay / shorta0
        // This is essentially y = y + 0x80
        y = (uint16_t)(y + 0x80);
        snes->cpu->y = y;

        counter++; // inc $aa
    }
    // Final state: A is set to 0 by shorta0 (clr_a) before loop exit
    snes->cpu->a = 0;
}

// PITFALLS: 7 (ASL truncation to 8-bit), 8 (Inherited battle mode mf=1, xf=0)
// HELPERS: clear_bit_emu(snes) — delegates ClearBit @ $00:855A
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=16-bit (target list base)
//   inputs_ram:  0xAB=1, 0xA9=1, 0xAD=1, 0x2003+y=1
//   output_ram:  0xAD=1, 0xAA=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::ClearInvalidTargets ($A7:4D)