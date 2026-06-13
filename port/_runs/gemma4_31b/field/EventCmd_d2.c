#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$EE, DP=0
// Logic:
//   Animates the mosaic effect over 64 frames.
//   Iterates a counter in ram[0x79], shifting it to index into EventMosaicTbl.
//   Each frame (Vblank), it updates the hardware mosaic register $2106.
//   Ends with a tail-call to WaitVblankEvent.
static void EventCmd_d2_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x79] = 0; // stz $79

loop_ee5e:;
    WaitVblankShort_emu(snes); // jsr WaitVblankShort

    uint8_t counter = ram[0x79];
    uint8_t index = (uint8_t)(counter >> 1); // lsr A / tax (Pitfall 7: 8-bit truncation)

    // ROM access for EventMosaicTbl. 
    // Note: In the snesrev pattern, ROM constants are accessed via a global table 
    // or specific ROM offset helper. Based on the project context:
    uint8_t mosaic_val = snes->rom_data[EVENT_MOSAIC_TBL + index];
    
    // Hardware register write $2106
    snes->io[0x2106] = mosaic_val;

    ram[0x79]++; // inc $79

    if (ram[0x79] != 0x40) { // cmp #$40 / bne @ee5e
        goto loop_ee5e;
    }

    // jmp WaitVblankEvent (Tail-call)
    WaitVblankEvent_emu(snes);
}

// PITFALLS: 7 (lsr in 8-bit mode), 8 (Inherited mf=true)
// HELPERS: WaitVblankShort_emu(snes), WaitVblankEvent_emu(snes)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x0079=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xEE
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_d2 ($EE:5C)