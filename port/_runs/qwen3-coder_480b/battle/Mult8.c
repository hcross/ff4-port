// Multiply two 8-bit values (signed or unsigned) using shift-and-add
//   in:  ram[$df] = multiplicand (8-bit), ram[$e1] = multiplier (8-bit)
//   out: ram[$394d] = product (16-bit LE), ram[$e3] = overflow byte
// Entry mode: A/X 16-bit (longa), DB=$7E, DP=0
static void Mult8_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0xE0] = 0;
    ram[0xE2] = 0;
    // longa: A and X now 16-bit
    uint16_t x = 0x0010;
    ram[0xE3] = 0;
    ram[0x394D] = 0;
    uint16_t acc = 0;  // accumulator for partial product

    do {
        uint8_t carry = (ram[0xE1] & 1);  // lsb of multiplier
        ram[0xE1] >>= 1;                  // ror $e1 (8-bit shift)
        if (carry) {
            acc += ram[0xDF];             // adc $394d (but we're building it)
        }
        // ror $394d / ror $e3: simulate 24-bit right shift on (acc, e3)
        uint8_t new_carry = acc & 1;
        acc >>= 1;
        if (ram[0xE3] & 1) acc |= 0x8000;  // carry from e3 into acc
        ram[0xE3] >>= 1;
        if (new_carry) ram[0xE3] |= 0x80;  // propagate carry to e3
        x--;
    } while (x != 0);

    write16(ram, 0x394D, acc);  // final result in 394D/394E
    // shorta0: back to 8-bit A
}

// PITFALLS: 6 (mode A is 16-bit due to `longa`), 7 (arithmetic truncation
// not needed here because we simulate 16-bit ops manually)
// HELPERS: none
// CONTRACT:
//   inputs_ram:  0xdf=1, 0xe1=1
//   output_ram:  0x394d=2
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::Mult8 ($00:83E0)