#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00, DP=0
// Logic:
//   1. Increments X and stores the 16-bit result into RAM $00B3.
//   2. Checks RAM $0081.
//   3. If RAM $0081 == 0: Loads byte from ($09D5 + X) and calls _00e9cf.
//   4. If RAM $0081 != 0: Calls _00ea14.
//   5. Ends by jumping to WaitVblankEvent.
static void EventCmd_f9_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // inx / stx $b3
    cpu->x++;
    write16(ram, 0x00B3, cpu->x);

    // lda $81 / bne @e9c9
    uint8_t val81 = ram[0x0081];
    if (val81 == 0) {
        // lda $09d5,x
        cpu->a = ram[0x09D5 + cpu->x];
        
        // jsr _00e9cf
        _00e9cf_emu(snes);
    } else {
        // jsr _00ea14
        _00ea14_emu(snes);
    }

    // jmp WaitVblankEvent
    WaitVblankEvent_emu(snes);
}

// PITFALLS: 1 (DB=$00), 6 (A 8-bit, X 16-bit)
// HELPERS: _00e9cf_emu(snes), _00ea14_emu(snes), WaitVblankEvent_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  0x0081=1, 0x09D5=1 (indexed by X)
//   output_ram:  0x00B3=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_f9 ($E9:B9)