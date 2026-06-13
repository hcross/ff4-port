#include "snes/snes.h"

// Handles event command f9: advances event index and processes based on $81
// If $81 != 0, calls _00ea14; otherwise reads from $09d5,x and calls _00e9cf
// Ends by waiting for vblank
static void EventCmd_f9_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    cpu->x++;                        // inx
    ram[0xB3] = cpu->x & 0xFF;       // stx $b3 (only low byte stored)
    uint8_t a = ram[0x81];           // lda $81
    if (a != 0) {                    // bne @e9c9
        _00ea14_emu(snes);           // jsr _00ea14
    } else {
        uint8_t arg = ram[0x09D5 + cpu->x]; // lda $09d5,x
        cpu->a = arg;                // jsr _00e9cf (pass arg in A)
        _00e9cf_emu(snes);
    }
    wait_vblank_event_emu(snes);     // jmp WaitVblankEvent
}

// PITFALLS: 8 (A/X mode inheritance — assumed 8-bit A from macro context)
// HELPERS: _00e9cf_emu, _00ea14_emu, wait_vblank_event_emu
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  0x81=1, 0x09d5=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_f9 ($E9:B9)