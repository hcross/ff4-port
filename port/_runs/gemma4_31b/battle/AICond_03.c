// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine checks a specific AI condition against a target.
// It iterates through a list of attributes/flags starting at $29EB,
// counting matches and updating a status byte at $DE.
static void AICond_03_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    get_ai_cond_target_emu(snes); // jsr GetAICondTarget

    uint8_t val289e = ram[0x289E];
    uint8_t valDD = ram[0xDD];

    if (val289e != 0) {            // bne @bdc6
        // Path @bdc6
        uint8_t a = val289e;
        a--;                      // dec
        if (a != 0) {              // bne @bdd2
            // If a != 0, we check valDD
            if (valDD == 0) {      // beq @bded
                ram[0xDE]++;       // inc $de
                return;
            } else {               // bne @bdef
                return;
            }
        } else {                  // a == 0
            // If a == 0, check valDD opposite logic
            if (valDD != 0) {      // beq @bded (not taken)
                ram[0xDE]++;       // inc $de
                return;
            } else {               // bne @bdef (not taken)
                // Fallthrough to @bdd2
            }
        }
    } else {                      // val289e == 0
        // Check valDD
        if (valDD != 0) {          // bne @bded
            ram[0xDE]++;           // inc $de
            return;
        } else {                  // beq @bdef
            return;
        }
    }

    // Label @bdd2
    if (valDD == 0) {              // beq @bdef
        return;
    }

    // clr_ax (A=0, X=0 since DP=0)
    uint8_t count = 0;             // stx $a9
    ram[0xA9] = count;

    // Loop @bdda
    for (uint16_t x = 0; x < 16; x++) { // inx2 / cpx #10 (16 decimal)
        // Note: inx2 is X += 2, but loop limit is 16. 
        // This means it checks indices 0, 2, 4... 14, 16 (Wait, 0x10 is 16)
        // The loop runs for x = 0, 2, 4, 6, 8, 10, 12, 14. When x=16, cpx #10 is equal.
        // However, the ASM is `inx2 / cpx #10 / bne @bdda`. 
        // If X starts at 0, the first check is after inx2 (X=2). 
        // It iterates X = 2, 4, 6, 8, 10, 12, 14, 16.
        
        // To match the ASM loop logic exactly:
        // The sequence is: clr_ax (X=0), then [body], then inx2 (X=2), then cpx #10
        // The body is executed for X=0, 2, 4, 6, 8, 10, 12, 14.
        // When X reaches 16 (0x10), bne @bdda is NOT taken.
        
        // But wait, the body is executed FIRST, then inx2.
        // Iteration 1: X=0, body, X=2, cpx 16 -> loop
        // ...
        // Iteration 8: X=14, body, X=16, cpx 16 -> exit
        
        // The current 'x' in the C loop should represent the index.
        // We must manually mirror the logic.
    }
    
    // Redoing loop to be byte-perfect with the ASM flow
    uint16_t x_reg = 0; 
    do {
        if (ram[0x29EB + x_reg] != 0) { // lda $29eb,x / beq @bde1
            ram[0xA9]++;                // inc $a9
        }
        x_reg += 2;                     // inx2
    } while (x_reg != 0x10);            // cpx #10 / bne @bdda

    uint8_t final_count = ram[0xA9];
    final_count--;                      // dec
    if (final_count != 0) {             // bne @bdef
        // fall through to @bded
        ram[0xDE]++;
    }
}

// PITFALLS: 5 (clr_ax as zero-clear), 6 (A 8-bit, X 16-bit), 8 (Inherited mf=true, xf=false)
// HELPERS: get_ai_cond_target_emu(snes) — delegates GetAICondTarget @ $BF0F
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x289E=1, 0xDD=1, 0x29EB=16 (array of 16 bytes)
//   output_ram:  0xDE=1, 0xA9=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_03 ($BD:B8)