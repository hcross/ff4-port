// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Checks AI conditions based on target data at $289E and status byte $DD.
// If conditions pass, it iterates through a flag array at $29EB and
// increments the success counter at $DE.
static void AICond_03_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    GetAICondTarget_emu(snes); // jsr GetAICondTarget

    uint8_t val289e = ram[0x289E];
    uint8_t valDD = ram[0xDD];

    if (val289e != 0) { // bne @bdc6
        uint8_t a = val289e;
        a--; // dec
        if (a != 0) { // bne @bdd2
            // @bdd2 path: logic continues below
        } else {
            // @bdc6 path (a == 0): check valDD
            if (valDD == 0) { // beq @bded
                ram[0xDE]++;
                return;
            } else { // bne @bdef
                return;
            }
        }
    } else {
        // @bdb8 path (val289e == 0): check valDD
        if (valDD != 0) { // bne @bded
            ram[0xDE]++;
            return;
        } else { // beq @bdef
            return;
        }
    }

    // Label @bdd2
    if (valDD == 0) { // beq @bdef
        return;
    }

    // clr_ax (A=0, X=0 since DP=0)
    ram[0xA9] = 0; // stx $a9

    // Loop @bdda
    uint16_t x_reg = 0;
    do {
        if (ram[0x29EB + x_reg] != 0) { // lda $29eb,x / beq @bde1
            ram[0xA9]++; // inc $a9
        }
        x_reg += 2; // inx2
    } while (x_reg != 0x10); // cpx #10 / bne @bdda

    uint8_t final_count = ram[0xA9];
    final_count--; // dec
    if (final_count != 0) { // bne @bdef
        // fall through to @bded
        ram[0xDE]++;
    }
}

// PITFALLS: 5 (clr_ax as zero-clear), 6 (A 8-bit, X 16-bit), 8 (Inherited mf=true, xf=false)
// HELPERS: GetAICondTarget_emu(snes) — delegates GetAICondTarget @ $BF0F
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x289E=1, 0xDD=1, 0x29EB=16 (array of 16 bytes)
//   output_ram:  0xDE=1, 0xA9=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_03 ($BD:B8)