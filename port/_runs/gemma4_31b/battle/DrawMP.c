// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$80, DP=0
// Logic:
//   Toggles a flag at $353E.
//   If the flag was 0 (now 1), it skips the logic and returns.
//   If the flag was 1 (now 0), it checks a counter at $353F.
//   If counter is 2, it resets it to 0.
//   If counter > 0, it triggers the battle graphics routine to draw MP ($0d).
//   Increments the counter.
static void DrawMP_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $353e / eor #1 / sta $353e
    ram[0x353E] ^= 1;

    // bne @8084 (branches if the NEW value of $353E is non-zero)
    if (ram[0x353E] != 0) {
        return;
    }

    // lda $353f / cmp #$02 / bne @8075
    if (ram[0x353F] == 0x02) {
        ram[0x353F] = 0; // lda #0 / sta $353f
    }

    // lda $353f / bne @807c
    if (ram[0x353F] != 0) {
        // lda #$0d / jsr ExecBtlGfx
        // We simulate the accumulator load by passing the value 
        // or setting it in the emulator state.
        snes->cpu->a = 0x0D; 
        exec_btl_gfx_emu(snes);
    }

    // inc $353f / @8084: rts
    ram[0x353F]++;
}

// PITFALLS: 1 (DB=$80 required), 7 (8-bit increment wraps naturally in C uint8_t)
// HELPERS: exec_btl_gfx_emu(snes) — delegates ExecBtlGfx (@8085)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x353E=1, 0x353F=1
//   output_ram:  0x353E=1, 0x353F=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x80
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::DrawMP ($80:5F)