// This function evaluates AI condition 00: checks if any entity in a target list
// satisfies a bitwise condition against a mask, and sets $de accordingly.
static void AICond_00_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    get_ai_cond_target_emu(snes);  // jsr GetAICondTarget
    if (ram[0xDD] == 0) return;    // lda $dd / beq @bd3c

    // Compute base address: $80 = #$03 + $289e
    uint16_t addr = 0x03 + read16(ram, 0x289E);  // clc / adc / sta chains
    write16(ram, 0x80, addr);

    uint16_t x = 0;  // clr_ax (A/X = DP = 0)
    do {
        uint16_t offset = read16(ram, 0x35D0 + x);  // lda ($35d0,x) pair
        write16(ram, 0xA9, offset);
        uint16_t y = read16(ram, offset);           // ldy $a9
        if (y == 0xFFFF) goto next;                 // cpy #$ffff / beq @bd28

        if ((ram[addr] & ram[0x289F]) != 0) {       // lda ($80) / and $289f / bne @bd1f
            ram[0xDE] = 1;                          // lda #$01 / sta $de
            if ((int8_t)ram[0x289D] >= 0) return;   // lda $289d / bpl @bd3c
        } else {
            if ((int8_t)ram[0x289D] < 0) {          // lda $289d / bpl / stz
                ram[0xDE] = 0;                      // stz $de
                return;
            }
        }
    next:
        addr += 0x80; write16(ram, 0x80, addr);     // longa / clc / adc / sta / shorta0
        x += 2;                                     // inx2
    } while (x != 0x1A);                            // cpx #$001a / bne @bcff
}

// PITFALLS: 1 (DB=0x7E assumed), 6 (A mode changes: 8-bit default, longa/shorta0),
// 8 (X is 16-bit due to longi in caller — clr_ax uses 16-bit X)
// HELPERS: get_ai_cond_target_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x289d=1, 0x289e=2, 0x289f=1, 0xDD=1
//   output_ram:  0xDE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: auto
REVERSED_FUNCTION: battle::AICond_00 ($BC:E8)