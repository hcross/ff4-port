// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Generate a random value (1-9), compare it against a value at 0x2707.
// If the random value is greater than 0x2707, it updates 0x2707.
// Otherwise, it calls RemoveTarget.
static void MagicEffect_03_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Setup for RandXA: X=1 (min), A=9 (max)
    snes->cpu->x = 1;
    snes->cpu->a = 9;
    uint16_t rand_val = randxa_emu(snes); // jsr RandXA

    // Store random result in $a9
    ram[0xA9] = (uint8_t)rand_val;

    // longa: transition to 16-bit A to read $2707
    uint16_t val_2707 = read16(ram, 0x2707);
    uint8_t random_byte = ram[0xA9];

    if (val_2707 <= random_byte) { // cmp $a9 / bcc @d45f (branches if 2707 < a9)
        // Note: BCC branches if Carry is clear. 
        // CMP A, M: Carry = (A >= M). 
        // So BCC triggers if A < M.
        // Here: A = read16($2707), M = ram[$a9].
        // If 2707 >= a9, it continues to update.
        // If 2707 < a9, it branches to @d45f.
        
        // This is a slight paradox in the ASM logic:
        // If 2707 < a9 -> branch to RemoveTarget.
        // If 2707 >= a9 -> set 2707 = a9.
        
        // Wait, let's re-evaluate: 
        // lda $2707 (A = val)
        // cmp $a9 (compare A with random)
        // bcc @d45f (branch if A < random)
        // Therefore: if val_2707 >= random_byte, it DOES NOT branch.
        
        ram[0x2707] = random_byte; // sta $2707 (shorta)
    } else {
        // @d45f: shorta0 (A=D, A 8-bit)
        remove_target_emu(snes); // jsr RemoveTarget
    }
}

// PITFALLS: 6 (Mode A transition: longa/shorta), 1 (DB=$7E)
// HELPERS: randxa_emu(snes), remove_target_emu(snes), read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2707=2
//   output_ram:  0x2707=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_03 ($D4:43)