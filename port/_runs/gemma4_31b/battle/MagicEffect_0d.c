// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine manages a specific magic effect counter (at $272A).
// If the counter reaches $FF, it removes the target.
// If the battle context is not "zeromus" ($1800=$B7, $1801=0), the counter is incremented.
static void MagicEffect_0d_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    uint8_t counter = ram[0x272A];
    if (counter == 0xFF) {              // cmp #$ff / bne @d849
        remove_target_emu(snes);        // jmp RemoveTarget
        return;
    }

    // Check if we are in the "zeromus" state
    if (ram[0x1800] == 0xB7 && ram[0x1801] == 0) { // lda $1800 / cmp #$b7 / lda $1801 / bne @d862
        return; // Branch to @d862 (RTS)
    }

    // Increment counter with 8-bit saturation at 0xFF
    uint8_t next = (uint8_t)(counter + 0x05); // adc #$05 (Pitfall 7: truncate to 8-bit)
    if (next < counter) {                 // bcc @d85f (Carry set if result > 255)
        next = 0xFF;
    }
    ram[0x272A] = next;                  // sta $272a
}

// PITFALLS: 7 (Arithmetic truncation: adc #$05 wrapped to uint8_t to match 8-bit A)
// HELPERS: remove_target_emu(snes) — delegates RemoveTarget @ $E030
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x272A=1, 0x1800=1, 0x1801=1
//   output_ram:  0x272A=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_0d ($D8:3F)