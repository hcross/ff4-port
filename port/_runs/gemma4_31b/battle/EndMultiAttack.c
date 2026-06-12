// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine scans a list of targets starting at $33C2.
// It counts how many valid targets exist until it hits $FC or $FF.
// If it hits $FC, it resets the target list to a specific 3-byte sequence.
//
// Logic:
//   - Loop through ram[$33C2 + x]
//   - If value == 0xFF: stop (end of list)
//   - If value == 0xFC: stop, increment count at $A9, and write reset sequence to $33C2
//   - Else: increment index and continue
static void EndMultiAttack_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // clr_ax: DP is 0, so A = 0, X = 0
    uint16_t x = 0;
    ram[0xA9] = 0; // stx $a9 (X is 0)

    while (1) {
        uint8_t val = ram[0x33C2 + x]; // lda $33c2,x
        if (val == 0xFF) {             // cmp #$ff / beq @b36b
            break;
        }
        if (val == 0xFC) {             // cmp #$fc / beq @b35a
            ram[0xA9]++;               // inc $a9
            ram[0x33C2] = 0xE1;        // lda #$e1 / sta $33c2
            ram[0x33C3] = 0xFC;        // lda #$fc / sta $33c3
            ram[0x33C4] = 0xFF;        // lda #$ff / sta $33c4
            break;
        }
        x++;                           // inx / bra @b34c
    }
}

// PITFALLS: 5 (clr_ax is tdc/tax, acts as zero-clear since DP=0)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x33C2=1, 0x33C3=1, 0x33C4=1
//   output_ram:  0x00A9=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::EndMultiAttack ($B3:48)