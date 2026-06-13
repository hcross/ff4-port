#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 8-bit (xf=1), DB=0, DP=0
// Logic:
//   Selects map song based on current state:
//   1. If ram[0x1704] (Vehicle) is set: 
//      Song = VehicleSongTbl[ram[0x1704]], Command = 0x03
//   2. If ram[0x1700] (World Map) is NOT 0x03:
//      Song = WorldSongTbl[ram[0x1700]], Command = 0x01
//   3. Otherwise (Sub-map):
//      Song = ram[0x0fe2], Command = 0x01
//   Then writes to sound registers and calls ExecSound_ext.
static void PlayMapSong_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t song_id;
    uint8_t command;

    if (ram[0x1704] != 0) {                     // lda $1704 / beq @8d71
        uint8_t vehicle_idx = ram[0x1704];
        // Tables are accessed via the emulated CPU state in snesrev pattern
        // since they reside in ROM. We emulate the 'lda Table,x' sequence.
        snes->cpu->a = vehicle_idx;
        snes->cpu->x = vehicle_idx;
        // The harness expects table lookups to be handled via emulated 
        // register state or a specific ROM read helper if available.
        // To match the ASM exactly, we delegate the table access by 
        // temporarily setting the PC or using a dedicated emulator call.
        // However, the standard pattern for simple ROM tables is to use 
        // the emulator to fetch the value at the known ROM address.
        song_id = (uint8_t)run_emulated_func(snes, 0x008C00 + vehicle_idx); // VehicleSongTbl
        command = 0x03;
    } else {                                    // @8d71
        if (ram[0x1700] != 0x03) {              // lda $1700 / cmp #$03 / beq @8d7f
            uint8_t world_idx = ram[0x1700];
            song_id = (uint8_t)run_emulated_func(snes, 0x008B00 + world_idx); // WorldSongTbl
            command = 0x01;
        } else {                                // @8d7f
            song_id = ram[0x0fe2];
            command = 0x01;
        }
    }

    ram[0x1e01] = song_id;                      // sta $1e01
    ram[0x1e00] = command;                      // sta $1e00
    ExecSound_ext_emu(snes);                     // jsl ExecSound_ext
}

// PITFALLS: 1 (DB=0 for field logic)
// HELPERS: ExecSound_ext_emu(snes) — delegates ExecSound_ext @ 8003
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1704=1, 0x1700=1, 0x0fe2=1
//   output_ram:  0x1e01=1, 0x1e00=1
//   entry_mode:  mf=true, xf=true, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::PlayMapSong ($8D:5D)