// AI condition 02: byte table-lookup equality test.
//   idx = ram[$289E]
//   val = ram[$35F3 + idx]      (8-bit table lookup, X used as 16-bit index)
//   if val == ram[$289F]:  ram[$DE]++
// Entry: mf=1 (8-bit A), xf=0 (X 16-bit), DB=$7E, DP=0.
//   No register I/O; the routine reads/writes only WRAM.
static void AICond_02_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t idx = ram[0x289E];
    uint8_t val = ram[0x35F3 + idx];          // lda $35f3,x (X=idx, high byte 0)
    if (val == ram[0x289F]) {                 // cmp $289f / bne (Pitfall 3)
        ram[0xDE]++;                          // inc $de
    }
}

// PITFALLS: 3 (cmp/bne — branch taken when A != mem, body runs when A == mem)
// HELPERS: (none)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x289E=1, 0x289F=1
//   output_ram:  0xDE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::AICond_02 ($BD:A9)