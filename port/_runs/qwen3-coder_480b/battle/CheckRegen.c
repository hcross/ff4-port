// Entry mode: A 16-bit on entry (inherited), X/Y 16-bit (xf=0)
// DB = $7E, DP = 0 (standard battle module convention)
// No register inputs; all state is in WRAM.
// Output: ram[$357C] = 0xFF if regen is disabled, unchanged otherwise
static void CheckRegen_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint16_t x = 0;  // clr_ax → tdc / tax, DP=0 → A/X = 0
    for (int y = 5; y > 0; y--) {
        uint8_t char_id = ram[0x2000 + x] & 0x1F;
        if (char_id != 0x13) {  // not Fusoya
            x += 0x80;          // advance to next character block
            continue;
        }
        // Found Fusoya
        uint8_t status1 = ram[0x2003 + x] & 0xC0;
        if (status1 != 0) goto disable_regen;  // dead or stone
        uint8_t status2 = ram[0x2004 + x] & 0x3C;
        if (status2 != 0) goto disable_regen;  // paralyze, sleep, charm, berserk
        return;  // regen allowed
    disable_regen:
        ram[0x357C] = 0xFF;  // disable regen
        return;
    }
}

// PITFALLS: 1 (DB must be $7E), 6 (A starts 16-bit due to longa in loop),
//           8 (A/X mode inherited — assumed 16-bit X from caller)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2000=1, 0x2003=1, 0x2004=1
//   output_ram:  0x357C=1
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::CheckRegen ($FF:B4)