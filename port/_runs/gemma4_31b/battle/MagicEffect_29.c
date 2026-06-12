// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$DD, DP=0
// Logic:
//   1. Increment battle state byte at $38E6.
//   2. Scan a table in the current bank ($DD) starting at $29B5.
//   3. Find the first byte equal to 0xFF.
//   4. Store the index (X) into RAM $008A.
//   5. Load A=1 and jump to ActivateMonster.
static void MagicEffect_29_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // inc $38e6
    ram[0x38E6]++;

    // clr_ax (tdc / tax)
    // DP=0 in battle, so X is zeroed.
    uint16_t x = 0;

    // Search loop: lda $29b5,x / cmp #$ff / beq @ddaa
    while (1) {
        // The table $29B5 is in Bank $DD. 
        // In this architecture, memory access for current bank (DB) 
        // is handled via the emulated memory read helper.
        uint8_t val = run_emulated_read8(snes, 0xDD29B5 + x);
        
        if (val == 0xFF) {
            break;
        }
        x++; // inx / bra @dda0
    }

    // stx $8a (X is 16-bit)
    write16(ram, 0x8A, x);

    // lda #$01
    cpu->a = 0x01;
    cpu->z = (cpu->a == 0);
    cpu->n = (cpu->a & 0x80) != 0;

    // jmp ActivateMonster
    ActivateMonster_emu(snes);
}

// PITFALLS: 5 (clr_ax treats DP as 0), 8 (mf=true inherited for battle module)
// HELPERS: ActivateMonster_emu(snes) — delegates ActivateMonster @ $DF5F
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x38E6=1, 0xDD29B5=1
//   output_ram:  0x8A=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xDD
//   entry_flags: z=auto, n=auto

REVERSED_FUNCTION: battle::MagicEffect_29 ($DD:9B)