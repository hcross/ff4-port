// Sets targeting parameters for a monster (slot 5, priority 0x0D) then jumps to shared logic
static void TargetMonster_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    ram[0xA9] = 0x05;  // Target slot 5
    ram[0xAB] = 0x0D;  // Priority 0x0D
    // Fall through to shared targeting logic
    _bfc9_emu(snes);
}

// PITFALLS: 1 (DB must be $7E for correct RAM writes)
// HELPERS: _bfc9_emu(snes) — delegates unresolved target logic
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0xa9=1, 0xab=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::TargetMonster ($C0:0061)