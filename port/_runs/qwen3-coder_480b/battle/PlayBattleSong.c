// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: cpu->a = song_id (8-bit)
// Logic:
//   1. Store song_id in $a9
//   2. Clear $1e00, $1e01, $1e05
//   3. Write song_id to $1e01, set $1e00 to 1
//   4. Call ExecSound_ext (delegated)
//   5. Loop until $1e05 == song_id AND $1e04 == 1
static void PlayBattleSong_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t song_id = snes->cpu->a;   // Entry: A = song_id
    ram[0xA9] = song_id;              // sta $a9
    ram[0x1E00] = 0;                  // stz $1e00
    ram[0x1E01] = 0;                  // stz $1e01
    ram[0x1E05] = 0;                  // stz $1e05

    do {
        ram[0x1E01] = song_id;        // lda $a9 / sta $1e01
        ram[0x1E00] = 1;              // lda #$01 / sta $1e00
        ExecSound_ext_emu(snes);      // jsl ExecSound_ext
        // Loop while $1e05 != song_id OR $1e04 != 1
    } while (ram[0x1E05] != song_id || ram[0x1E04] != 1);
}

// PITFALLS: 1 (DB=$7E required for WRAM access)
// HELPERS: ExecSound_ext_emu(snes) — delegates ExecSound_ext @ $8003
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
REVERSED_FUNCTION: battle::PlayBattleSong ($FF:12)