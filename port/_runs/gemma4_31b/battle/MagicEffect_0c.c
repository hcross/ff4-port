// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Logic: 
//   Checks target and caster flags ($28A3).
//   If ($272B & $28A3) != 0 OR 
//      ($2703 & $28A3) != 0 OR 
//      ($2703 >= $28A3), 
//   then call RemoveTarget.
//   Otherwise, flip bits in $2703 defined by $28A3 and return.
static void MagicEffect_0c_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $272b / and $28a3 / bne @d832
    if ((ram[0x272B] & ram[0x28A3]) != 0) {
        remove_target_emu(snes);
        return;
    }

    // lda $2703 / and $28a3 / bne @d835
    if ((ram[0x2703] & ram[0x28A3]) != 0) {
        // Jump to @d835 (Logic: flip bits in $2703)
        ram[0x2703] ^= ram[0x28A3];
        return;
    }

    // lda $2703 / cmp $28a3 / bcc @d835
    // Pitfall 3: bcc branches when A < ram[$28A3]
    if (ram[0x2703] < ram[0x28A3]) {
        // Jump to @d835
        ram[0x2703] ^= ram[0x28A3];
        return;
    }

    // Fallthrough to @d832: jmp RemoveTarget
    remove_target_emu(snes);
}

// PITFALLS: 3 (CMP/BCC: bcc branches when A < mem, so logic flows to 
// RemoveTarget when A >= mem)
// HELPERS: remove_target_emu(snes) — delegates RemoveTarget @ $E030
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x272B=1, 0x28A3=1, 0x2703=1
//   output_ram:  0x2703=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_0c ($D8:1A)