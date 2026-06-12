// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: A = value to store at $82, Y = value to store at $80
// Logic:
//   Store A to $82, Y to $80.
//   If $e5 == 0, return.
//   Else, scan bytes at [$80] with index Y until #$ff is found.
//   For each match, decrement $e5. If $e5 reaches zero, stop and leave Y at
//   the next byte. Otherwise, continue scanning.
static void GetScriptPtr_c(Snes *snes, uint8_t a, uint16_t y) {
    uint8_t *ram = snes->ram;
    ram[0x82] = a;
    write16(ram, 0x80, y);
    uint16_t addr = y;
    uint8_t e5 = ram[0xE5];
    if (e5 == 0) return; // beq @845d
    uint16_t y_idx = 0;
    for (;;) {
        uint8_t byte = ram[addr + y_idx]; // lda [$80],y
        if (byte == 0xFF) {               // cmp #$ff
            e5--;
            ram[0xE5] = e5;
            if (e5 == 0) {
                y_idx++; // iny (final)
                break;
            }
        }
        y_idx++; // iny
        // bra @844d (loop)
    }
    // Y register is not used after, so no need to update CPU Y
}

// PITFALLS: 1 (DB=$7E assumed), 8 (mode A 8-bit, X 16-bit)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=16
//   inputs_ram:  0x00E5=1
//   output_ram:  0x00E5=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::GetScriptPtr ($84:43)