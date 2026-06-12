// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Iterates through a script array starting at the address in Y, 
// searching for a terminator ($FF) or until the counter in $E5 reaches 0.
// It stores the current A and Y values into RAM before processing.
static void GetScriptPtr_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // sta $82 / sty $80
    ram[0x82] = (uint8_t)snes->cpu->a;
    write16(ram, 0x80, snes->cpu->y);

    // clr_ay (tdc / tay) -> A = 0, Y = 0 (since DP=0)
    uint16_t y = 0;
    
    // lda $e5 / beq @845d
    uint8_t count = ram[0xE5];
    if (count == 0) goto exit_845d;

    // Loop to find terminator
    // lda [$80],y
    uint16_t base_addr = read16(ram, 0x80);
    
    do {
        uint8_t val = ram[base_addr + y];
        
        if (val == 0xFF) { // cmp #$ff / bne @8459
            // dec $e5 / lda $e5 / beq @845c
            ram[0xE5]--;
            if (ram[0xE5] == 0) {
                y++; // iny @845c
                goto exit_845d;
            }
        } else {
            // bne @8459 (taken if val != 0xFF)
            y++; // iny @8459
            continue; // bra @844d
        }
        
        // If val was $FF but ram[0xE5] was not 0, it continues to iny @8459
        y++; // iny @8459
    } while (true);

exit_845d:
    snes->cpu->y = y;
}

// PITFALLS: 5 (clr_ay is tdc/tay, effectively zeroing A and Y in DP=0),
// 6 (A is 8-bit, Y is 16-bit), 8 (inherited battle mode mf=true, xf=false).
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=16
//   inputs_ram:  0x0080=2, 0x00E5=1, [0x0080+y]=1
//   output_ram:  0x0082=1, 0x0080=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
//   custom_spike: yes (output is essentially Y register and RAM side-effects)
REVERSED_FUNCTION: battle::GetScriptPtr ($84:43)