#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// This routine executes a loop calling two sub-routines until a 
// bitmask in ram[0x7A] is cleared and a counter in ram[0x24] reaches 0x30.
// If the condition is met, it jumps to WaitVblankEvent.
static void Special_29_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    do {
        _00cf98_emu(snes); // jsr _00cf98
        _00cfb4_emu(snes); // jsr _00cfb4
        
        uint8_t status = ram[0x7A]; 
        if ((status & 0x07) == 0) { // lda $7a / and #$07 / bne @ce07
            ram[0x24]++;            // inc $24
            if (ram[0x24] == 0x30) { // lda $24 / cmp #$30 / bne @ce07
                wait_vblank_event_emu(snes); // jmp WaitVblankEvent
                return;
            }
        }
        // Loop continues if (ram[0x7A] & 0x07) != 0 OR ram[0x24] != 0x30
    } while (1); 
}

// PITFALLS: 1 (DB=$7E assumed for battle/field logic), 3 (CMP used for equality check)
// HELPERS: _00cf98_emu(snes), _00cfb4_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x7A=1, 0x24=1
//   output_ram:  0x24=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::Special_29 ($CE:04)