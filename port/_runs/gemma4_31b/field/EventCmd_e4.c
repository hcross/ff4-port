#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0xEB, DP=0
// Logic:
//   1. Fetch a byte from the event stream.
//   2. Iterate through a 320-byte buffer (0x1003 to 0x1142).
//   3. OR each byte in that buffer with the fetched byte.
//   4. Call NextChar after each update.
//   5. Set a flag in RAM [0xCC] and jump to WaitVblankEvent.
static void EventCmd_e4_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // jsr GetNextEventByte
    uint8_t event_byte = get_next_event_byte_emu(snes);
    
    // sta $06
    ram[0x06] = event_byte;

    // ldx #0 / loop @ebd8
    for (uint16_t x = 0; x <= 0x0140; x++) {
        // lda $1003,x
        // ora $06
        // sta $1003,x
        // Note: $1003 is absolute addressing. 
        // Offset is 0x1003 + x.
        ram[0x1003 + x] |= ram[0x06];

        // jsr NextChar
        next_char_emu(snes);
        
        // cpx #$0140 / bne @ebd8
        // Loop continues until X reaches 0x140.
        // Since we use a for-loop, the check is implicit.
    }

    // lda #$01 / sta $cc
    ram[0xCC] = 0x01;

    // jmp WaitVblankEvent
    wait_vblank_event_emu(snes);
}

// PITFALLS: 6 (Mode A 8-bit used for OR operation), 8 (Inherited mf=true 
// for event command handlers).
// HELPERS: get_next_event_byte_emu, next_char_emu, wait_vblank_event_emu
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0xCC=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xEB
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_e4 ($EB:D0)