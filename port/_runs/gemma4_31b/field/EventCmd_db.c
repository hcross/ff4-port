#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$EB, DP=0
// Purpose: XORs a block of memory ($1003 to $1142) with a byte fetched from 
// the event stream, then advances the character pointer and waits for Vblank.
static void EventCmd_db_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // jsr GetNextEventByte
    uint8_t event_byte = get_next_event_byte_emu(snes);
    
    // sta $06
    ram[0x06] = event_byte;
    
    // ldx #0 / loop @eb9a
    for (uint16_t x = 0; x <= 0x0140; x++) {
        // lda $1003,x / eor $06 / sta $1003,x
        // Note: $1003 is DP relative (DP=0)
        ram[0x1003 + x] ^= ram[0x06];
        
        // jsr NextChar
        next_char_emu(snes);
    }
    
    // lda #$01 / sta $cc
    ram[0xCC] = 0x01;
    
    // jmp WaitVblankEvent
    wait_vblank_event_emu(snes);
}

// PITFALLS: 6 (Mode A 8-bit), 8 (Inherited battle/field module mode mf=true)
// HELPERS: get_next_event_byte_emu(snes), next_char_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x1003=1 (modified range 0x1003-0x1143), 0xCC=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xEB
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_db ($EB:92)