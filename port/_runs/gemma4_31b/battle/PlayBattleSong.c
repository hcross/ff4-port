// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Polls ExecSound_ext until the requested song (A) is successfully playing.
//   - Stores requested song ID in RAM $a9.
//   - Sets sound request buffer ($1e00-$1e01).
//   - Calls ExecSound_ext and waits for status bytes $1e05 and $1e04 to match.
static void PlayBattleSong_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t song_id = (uint8_t)snes->cpu->a;

    ram[0xA9] = song_id;       // sta $a9
    ram[0x1E00] = 0;           // stz $1e00
    ram[0x1E01] = 0;           // stz $1e01
    ram[0x1E05] = 0;           // stz $1e05

    do {
        ram[0x1E01] = ram[0xA9]; // lda $a9 / sta $1e01
        ram[0x1E00] = 0x01;      // lda #$01 / sta $1e00
        
        ExecSound_ext_emu(snes); // jsl ExecSound_ext

        // Check if song is now playing: ram[$1e05] == requested song
        if (ram[0x1E05] == ram[0xA9]) { // lda $1e05 / cmp $a9 / bne @ff1d
            // Check if status $1e04 == 1
            if (ram[0x1E04] == 0x01) { // lda $1e04 / cmp #$01 / bne @ff1d
                break; 
            }
        }
    } while (true);
}

// PITFALLS: 1 (DB=$7E), 3 (CMP/BNE logic: loop continues while values are NOT equal)
// HELPERS: ExecSound_ext_emu(snes) — delegates ExecSound_ext @ $80:0003
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto

REVERSED_FUNCTION: battle::PlayBattleSong ($FF:12)