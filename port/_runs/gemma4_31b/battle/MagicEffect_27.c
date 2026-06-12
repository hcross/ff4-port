// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$DD, DP=0
// Logic:
// Copies a 16-bit value from $2709-$270A to $3945-$3946 and 
// writes a constant 10 (0x000A) to $3947.
// Then branches to the common handler at _dd77.
static void MagicEffect_27_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $2709 / sta $3945
    ram[0x3945] = ram[0x2709];
    // lda $270a / sta $3946
    ram[0x3946] = ram[0x270A];
    
    // ldx #$000a / stx $3947
    // X is 16-bit (xf=0), so stx writes 2 bytes.
    write16(ram, 0x3947, 0x000A);

    // bra _dd77
    _dd77_emu(snes); 
}

// PITFALLS: 1 (DB=$DD used for source $2709, targets are WRAM $7E), 
// 6 (Mode X is 16-bit, affecting stx $3947 width)
// HELPERS: _dd77_emu(snes) — delegates common effect handler at $DD:77

// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2709=1, 0x270A=1
//   output_ram:  0x3945=1, 0x3946=1, 0x3947=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xDD
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_27 ($DD:51)