#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$E6, DP=0
// Logic:
//   1. Fetch the next byte from the event stream.
//   2. Store that byte into RAM $1E01 (Sound ID).
//   3. Store constant $04 into RAM $1E00 (Sound Command).
//   4. Execute the sound command.
//   5. Jump to WaitVblankEvent to synchronize with the screen refresh.
static void EventCmd_ea_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // jsr GetNextEventByte
    // The result of GetNextEventByte is returned in the accumulator (A)
    uint16_t event_byte = GetNextEventByte_emu(snes);
    
    // sta $1e01
    ram[0x1E01] = (uint8_t)event_byte;
    
    // lda #$04 / sta $1e00
    ram[0x1E00] = 0x04;
    
    // jsl ExecSound_ext
    ExecSound_ext_emu(snes);
    
    // jmp WaitVblankEvent
    WaitVblankEvent_emu(snes);
}

// PITFALLS: None relevant for this linear sequence.
// HELPERS: GetNextEventByte_emu(snes), ExecSound_ext_emu(snes), WaitVblankEvent_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x1E00=1, 0x1E01=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE6
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_ea ($E6:01)