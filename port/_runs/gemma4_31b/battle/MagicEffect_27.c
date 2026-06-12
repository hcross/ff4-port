// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$DD (Bank $DD), DP=0
// Logic:
// Copies a 16-bit value from $2709-$270A to $3945-$3946 and 
// writes a constant 10 (0x000A) to $3947.
// Note: The routine ends with a branch to _dd77, which is treated
// as a fall-through or a call to the common effect handler.
static void MagicEffect_27_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $2709 / sta $3945
    ram[0x3945] = ram[0x2709];
    // lda $270a / sta $3946
    ram[0x3946] = ram[0x270A];
    
    // ldx #$000a / stx $3947
    // X is 16-bit (xf=0), but target $3947 is often treated as 8-bit 
    // in these effect tables or the following logic handles the word.
    // To match ASM 'stx $3947' with xf=0:
    write16(ram, 0x3947, 0x000A);

    // bra _dd77: The branch target is the common exit/execution 
    // logic for magic effects. We delegate the remaining sequence.
    magic_effect_common_emu(snes); 
}

// PITFALLS: 1 (DB=$DD used for source $2709, but target $3945 is WRAM $7E)
// Note: In the original SNES memory map, $2709 is likely in a 
// character/data bank ($DD) while $3945 is WRAM. The C implementation 
// uses snes->ram which covers the mapped WRAM. If $2709 is in ROM, 
// a read_rom helper would be used, but based on provided context 
// and snesrev patterns, these are indexed via the ram pointer.

// HELPERS: magic_effect_common_emu(snes) — delegates _dd77 sequence

// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2709=1, 0x270A=1
//   output_ram:  0x3945=1, 0x3946=1, 0x3947=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xDD
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::MagicEffect_27 ($DD:51)