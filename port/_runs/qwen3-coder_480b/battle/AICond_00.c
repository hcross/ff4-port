// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// All inputs/outputs in WRAM (no register I/O — convention battle):
//   in : ram[$289D] = condition flags, ram[$289E] = offset low,
//        ram[$289F] = mask, $35D0-$35EF = target list (word pairs)
//   out: ram[$DE] = result (0 or 1)
//
// Logic:
//   - GetAICondTarget sets up target data
//   - if ram[$DD] == 0, return
//   - compute base address: $80 = #$03 + ram[$289E], $81 = #$20 + carry
//   - iterate over targets at $35D0,x (word pairs)
//   - for each target:
//       if target == #$FFFF, skip
//       load byte from ($80), AND with mask
//       if result != 0, check ram[$289D] sign
//           if ram[$289D] < 0, set ram[$DE] = 0 and return
//           else set ram[$DE] = 1 and check ram[$289D] again
//       if ram[$289D] >= 0, continue loop
//   - if no match, increment $80 by #$80 (16-bit), loop until x==$001A
static void AICond_00_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    get_ai_cond_target_emu(snes);  // jsr GetAICondTarget
    if (ram[0xDD] == 0) return;    // lda $dd / beq @bd3c

    // Compute base address: $80 = #$03 + ram[$289E], $81 = #$20 + carry
    uint16_t base = 0x03 + ram[0x289E];
    uint8_t carry = (base > 0xFF) ? 1 : 0;
    ram[0x80] = base & 0xFF;
    ram[0x81] = 0x20 + carry;

    uint16_t x = 0;  // clr_ax (X = 0)
    do {
        uint16_t target_addr = read16(ram, 0x35D0 + x);
        write16(ram, 0xA9, target_addr);  // sta $a9 / lda $35d1,x / sta $aa
        if (target_addr == 0xFFFF) goto next_target;  // cpy #$ffff / beq

        // Load byte from ($80) and AND with mask
        uint16_t addr_80 = read16(ram, 0x80);
        uint8_t val = ram[addr_80];       // lda ($80)
        if ((val & ram[0x289F]) != 0) {   // and $289f / bne @bd1f
            if ((int8_t)ram[0x289D] < 0) { // lda $289d / bpl (inverted)
                ram[0xDE] = 0;             // stz $de
                return;
            }
            ram[0xDE] = 1;                 // lda #$01 / sta $de
            if ((int8_t)ram[0x289D] >= 0) { // lda $289d / bpl @bd3c
                return;
            }
        } else {
            if ((int8_t)ram[0x289D] >= 0) goto next_target; // bpl @bd28
        }

    next_target:
        // longa / clc / adc #$0080 / sta $80 / shorta0
        uint16_t new_addr_80 = read16(ram, 0x80) + 0x80;
        write16(ram, 0x80, new_addr_80);

        x += 2;  // inx2
    } while (x != 0x1A);  // cpx #$001a / bne @bcff

    // Fallthrough to RTS
}

// PITFALLS: 1 (DB=$7E required for absolute addressing),
//           3 (CMP/BPL sign check on $289D treated as signed int8),
//           6 (mode A 8-bit assumed throughout except for longa block),
//           8 (mode A/X inherited from caller — battle uses mf=1, xf=0)
// HELPERS: get_ai_cond_target_emu(snes) — delegates GetAICondTarget @ $BF0F
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x289D=1, 0x289E=1, 0x289F=1, 0x35D0=2
//   output_ram:  0x00DE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_00 ($BC:E8)