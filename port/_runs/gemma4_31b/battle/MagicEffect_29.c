// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7D (Bank $DD), DP=0
// Logic:
//   1. Increment value at $38E6
//   2. Search for the first occurrence of 0xFF in the table starting at $29B5 + offset(X)
//   3. Save the offset (index) to $8A
//   4. Set A = 1 and jump to ActivateMonster
static void MagicEffect_29_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // inc $38e6
    ram[0x38E6]++;

    // clr_ax (tdc / tax)
    // In battle context DP=0, so this zeros A and X.
    uint16_t x = 0;

    // Search loop
    while (1) {
        // lda $29b5,x (Offset addressing in bank $DD)
        // Since DB=$7D, the address is 0x7D0000 + 0x29B5 + x
        // However, the snesrev/zelda3 pattern handles WRAM/ROM mapping via the emulator.
        // For a translate mode, we access the memory based on the defined DB.
        // Note: $29B5 is in the ROM bank $DD. We use a helper or direct offset if mapped.
        uint8_t val = snes->rom[0xDD0000 + 0x29B5 + x]; // Assuming rom[] access for bank $DD
        
        if (val == 0xFF) { // cmp #$ff / beq @ddaa
            break;
        }
        x++; // inx / bra @dda0
    }

    // stx $8a (DP=0, so write to WRAM $008A)
    write16(ram, 0x8A, x);

    // lda #$01 / jmp ActivateMonster
    cpu->a = 0x01;
    cpu->z = (cpu->a == 0);
    cpu->n = (cpu->a & 0x80) != 0;
    
    activate_monster_emu(snes);
}

// PITFALLS: 5 (clr_ax as zero-clear), 8 (mf=true inherited for battle)
// HELPERS: activate_monster_emu(snes) — delegates ActivateMonster @ $DF5F
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x38E6=1, 0xDD29B5=1 (ROM search table)
//   output_ram:  0x8A=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7D
//   entry_flags: z=auto, n=auto

REVERSED_FUNCTION: battle::MagicEffect_29 ($DD:9B)