// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine validates if a character is eligible for an action.
// It checks if the character index is valid (not 0xFF) and verifies 
// that the character is not dead, petrified, or under specific status effects.
//
// RAM inputs: 
//   - 0x00D0: Character Index
//   - 0x2003+X: Status Flags 1 (Dead/Stone)
//   - 0x2004+X: Status Flags 2 (Paralyze/Sleep/Charm/Berserk)
//   - 0x2005+X: Status Flags 3 (Magnetized/Stopped/Twin/Jump)
// RAM output:
//   - 0x00D0: Set to 0xFF if validation fails.
static void ValidateChar_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    uint8_t char_idx = ram[0x00D0];
    if (char_idx == 0xFF) {              // cmp #$ff / beq @a5ad
        ram[0x00D0] = 0xFF;
        return;
    }

    select_obj_emu(snes);                // jsr SelectObj
    
    // SelectObj updates ram[0xA6] with the object offset/index
    uint16_t x = read16(ram, 0x00A6); 

    // Check Dead (0x80) or Stone (0x40)
    uint8_t status1 = ram[0x2003 + x];  // lda $2003,x
    if ((status1 & 0xC0) != 0) {         // and #$c0 / bne @a5ad
        ram[0x00D0] = 0xFF;
        return;
    }

    // Check Paralyze, Sleep, Charm, Berserk (0x3C)
    uint8_t status2 = ram[0x2004 + x];   // lda $2004,x
    if ((status2 & 0x3C) != 0) {         // and #$3c / bne @a5ad
        ram[0x00D0] = 0xFF;
        return;
    }

    // Check Magnetized, Stopped, Twin, Jump (0xC6)
    uint8_t status3 = ram[0x2005 + x];   // lda $2005,x
    if ((status3 & 0xC6) == 0) {         // and #$c6 / beq @a5b1
        return;                          // Character is valid (matches beq @a5b1)
    }

    // Fallthrough to failure state
    ram[0x00D0] = 0xFF;                  // lda #$ff / sta $d0
}

// PITFALLS: 1 (DB=$7E), 8 (mf=true assumed for battle code)
// HELPERS: select_obj_emu(snes) — delegates SelectObj @ $03:8489, read16
//
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00D0=1, 0x00A6=2, 0x2003=1, 0x2004=1, 0x2005=1
//   output_ram:  0x00D0=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::ValidateChar ($A5:8D)