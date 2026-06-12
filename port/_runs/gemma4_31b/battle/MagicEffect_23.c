// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine handles a specific magic effect: 
// 1. Sets a status/timer value at $28A4 to 0x20
// 2. Executes the Sleep/Paralyze logic
// 3. Jumps to a common effect exit handler at _d3e4
static void MagicEffect_23_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x28A4] = 0x20;                    // lda #$20 / sta $28a4
    sleep_paralyze_effect_emu(snes);      // jsr SleepParalyzeEffect
    
    // The original code ends with 'jmp _d3e4'. In a C reimplementation,
    // this is a tail-call. Since _d3e4 is likely a shared cleanup/return 
    // routine, we delegate it to maintain exact program flow.
    run_emulated_func(snes, 0x03D3E4u);    // jmp _d3e4
}

// PITFALLS: 1 (DB=$7E implicitly required for WRAM access at $28A4)
// HELPERS: sleep_paralyze_effect_emu(snes) — delegates SleepParalyzeEffect @ $D5A6
//          run_emulated_func — handles the tail-jump to _d3e4
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x28A4=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto

REVERSED_FUNCTION: battle::MagicEffect_23 ($DC:FB)