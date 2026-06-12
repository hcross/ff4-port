// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$E1, DP=0
// Logic:
//   Checks elemental attributes of the attacker and target.
//   - If attacker is strong vs target:
//     - Damage is 0 ($38FE = 0).
//     - If target is "healing" type (bit 0x40 set), set $38FE to 0x84 (2x HP restore).
//   - If target is strong vs attacker:
//     - Damage is halved ($38FE = 1).
//     - If attacker is "healing" type (bit 0x40 set), set $38FE to 0x82 (1x HP restore).
//   - Otherwise, no multiplier is set.
static void CheckStrongElem_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $2726 / and $28a2 / beq @e119
    // Note: $2726 and $28a2 are in the current data bank ($E1)
    // Since we use absolute offsets in snes->ram (WRAM), we must ensure 
    // the address calculation handles the bank if it's outside 0x7E.
    // However, in this specific engine, these are likely fixed offsets.
    if ((ram[0x2726] & ram[0x28A2]) != 0) {
        ram[0x38FE] = 0; // clr_a / sta $38fe (Pitfall 5: clr_a is tdc, DP=0)
        
        if ((ram[0x2726] & 0x40) != 0) { // lda $2726 / and #$40 / beq @e132
            ram[0x38FE] = 0x84;        // lda #$84 / sta $38fe
        }
        return;
    }

    // @e119: lda $2725 / and $28a2 / beq @e132
    if ((ram[0x2725] & ram[0x28A2]) != 0) {
        ram[0x38FE] = 0x01;            // lda #$01 / sta $38fe
        
        if ((ram[0x2725] & 0x40) != 0) { // lda $2725 / and #$40 / beq @e132
            ram[0x38FE] = 0x82;        // lda #$82 / sta $38fe
        }
    }
}

// PITFALLS: 1 (DB=$E1 for $27xx range), 5 (clr_a behaves as 0 because DP=0)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2726=1, 0x28A2=1, 0x2725=1
//   output_ram:  0x38FE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE1
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::CheckStrongElem ($E1:00)