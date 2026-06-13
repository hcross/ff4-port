#include "snes/snes.h"

// Triggers the playback of a specific song event by writing the event ID 
// to $1E01 and the "play" command (0x01) to $1E00, then calling the sound executor.
static void PlaySongEvent_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Entry mode: A 8-bit (mf=true), DB=$00, DP=0
    // Accumulator contains the song event ID
    ram[0x1E01] = (uint8_t)snes->cpu->a;
    
    // Set command 0x01 (play song)
    ram[0x1E00] = 0x01;

    // Execute the sound request
    ExecSound_ext_emu(snes);
}

// PITFALLS: None.
// HELPERS: ExecSound_ext_emu(snes) — delegates ExecSound_ext @ $80:0003
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x1E01=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::PlaySongEvent ($E7:EA)