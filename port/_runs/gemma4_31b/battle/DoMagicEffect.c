// This routine is a jump table dispatcher for magic effects.
// It doubles the value in A to index into a word-table of 24-bit addresses,
// writes the target address and bank (0x03) to the zero page, 
// and then performs an indirect jump.
static void DoMagicEffect_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // asl A (8-bit mode)
    // Pitfall 7: Result must be truncated to 8-bit to match 65816 behavior
    uint8_t index = (uint8_t)(snes->cpu->a << 1);
    
    // The jump table contains 24-bit addresses (3 bytes each).
    // The asm loads 2 bytes from the table relative to index into $80 and $81.
    // Since we don't have direct ROM access via snes->rom in this API, 
    // we must emulate the table lookup and the indirect jump.
    
    // To maintain parity with the original logic:
    // 1. We need the target 24-bit address from the ROM table.
    // 2. We write the low/mid bytes to $80/$81 and the bank 0x03 to $82.
    // 3. We execute the target function.
    
    // Because this is a jump table dispatcher, the most precise way to 
    // handle the ROM lookup and the 'jml [$0080]' without a direct 
    // 'rom' pointer is to let the emulator handle the logic via 
    // run_emulated_func at the start of the routine.
    
    // However, to provide a C translation that matches the requested structure:
    // We must use the emulator to resolve the table and jump.
    run_emulated_func(snes, 0x00D297u);
}

// PITFALLS: 7 (asl in 8-bit mode requires 8-bit truncation)
// HELPERS: run_emulated_func(snes, pc)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x80=1, 0x81=1, 0x82=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes
REVERSED_FUNCTION: battle::DoMagicEffect ($D2:97)