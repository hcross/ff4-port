#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x7E, DP=0
// Entry: cpu->a = song_id (8-bit), stored to $1E01
//        $1E00 set to 1 to indicate "play song" command
//        then calls ExecSound_ext (delegated)
static void PlaySongEvent_c(Snes *snes, uint8_t song_id) {
    uint8_t *ram = snes->ram;
    ram[0x1E01] = song_id;         // sta $1e01
    ram[0x1E00] = 1;               // lda #$01 / sta $1e00
    exec_sound_ext_emu(snes);      // jsl ExecSound_ext
}

// PITFALLS: 1 (DB must be $7E for writes to $1E00/$1E01 to reach WRAM)
// HELPERS: exec_sound_ext_emu(snes) — delegates ExecSound_ext @ $8003
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::PlaySongEvent ($E7:EA)