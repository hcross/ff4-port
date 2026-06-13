#include "snes/snes.h"

// Executes event command $EA: play sound then wait for vblank.
// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// No input registers; all state derived from GetNextEventByte and RAM.
static void EventCmd_ea_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t next_byte = get_next_event_byte_emu(snes); // jsr GetNextEventByte
    ram[0x1E01] = next_byte;                           // sta $1e01
    ram[0x1E00] = 0x04;                                // lda #$04 / sta $1e00
    exec_sound_ext_emu(snes);                          // jsl ExecSound_ext
    wait_vblank_event_emu(snes);                       // jmp WaitVblankEvent
}

// PITFALLS: 1 (DB must be $7E for WRAM access)
// HELPERS: get_next_event_byte_emu, exec_sound_ext_emu, wait_vblank_event_emu
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_ea ($E6:01)