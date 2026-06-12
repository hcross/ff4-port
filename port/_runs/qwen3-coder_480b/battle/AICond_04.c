// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// All inputs/outputs in WRAM (no register I/O — convention battle):
//   in : ram[$289F], ram[$29AD..$29AF], ram[$29CA..$29CC], ram[$29CD], ram[$289E]
//   out: ram[$A9] (0, 1, or 2), ram[$DE] (incremented if condition matches)
//
// Logic:
//   X = 0..2 loop:
//     if ram[$289F] == ram[$29AD + X]:
//       if ram[$29CA + X] != 0 and ram[$29CA + X] == ram[$29CD]:
//         ram[$A9] += 2
//       else:
//         ram[$A9] += 1
//   if ram[$289E] == ram[$A9]:
//     ram[$DE] += 1
static void AICond_04_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0xA9] = 0;                    // clr_ax / stx $a9

    for (uint16_t x = 0; x < 3; x++) { // inx / cpx #3 / bne loop
        if (ram[0x289F] == ram[0x29AD + x]) { // cmp $29ad,x / beq @be06
            uint8_t val = ram[0x29CA + x];
            if (val != 0 && val == ram[0x29CD]) { // cmp $29cd / bne @be14
                ram[0xA9] += 2;       // inc $a9 / inc $a9
            } else {
                ram[0xA9] += 1;       // inc $a9
            }
            break; // Exit loop after match (as in asm)
        }
    }

    if (ram[0x289E] == ram[0xA9]) {   // cmp $a9 / bne @be1d
        ram[0xDE]++;                  // inc $de
    }
}

// PITFALLS: 8 (X/Y 16-bit mode assumed from context — battle module default xf=0)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x289F=1, 0x29AD=1, 0x29AE=1, 0x29AF=1, 0x29CA=1, 0x29CB=1, 0x29CC=1, 0x29CD=1, 0x289E=1
//   output_ram:  0xA9=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_04 ($BD:F0)