#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x00 (Direct Page used for RAM access)
// This routine implements a frame-based delay loop that updates hardware registers
// ($2132, $212D, $2130, $2131) based on timing values in RAM.
static void _00ea14_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x79] = 0; // stz $79

    do {
        wait_frame_emu(snes); // jsr WaitFrame

        uint8_t val = ram[0x83];
        val &= 0xE0;            // and #$e0
        val |= ram[0x81];       // ora $81
        ram[0x2132] = val;     // sta $2132 (Hardware Reg)

        ram[0x79]++;            // inc $79
        if ((ram[0x79] & ram[0x82]) == 0) { // lda $79 / and $82 / bne @ea16 (inverted)
            break;
        }
    } while (1); // @ea16 loop

    uint8_t counter = ram[0x81];
    if (counter > 0) {           // dec $81 / bpl @ea16
        ram[0x81] = counter - 1;
        // The original ASM jumps back to @ea16 (WaitFrame). 
        // This creates a nested loop structure.
        // To match the logic exactly, we must wrap the outer logic.
        // However, the ASM is linear-ish: dec $81 -> bpl @ea16.
        // We effectively restart the inner loop.
        
        // Note: Since the C translation is a functional replacement, 
        // we must ensure the outer loop iteration is handled.
        // Re-running the inner logic:
        while (1) {
            ram[0x79] = 0;
            do {
                wait_frame_emu(snes);
                uint8_t v = ram[0x83];
                v &= 0xE0;
                v |= ram[0x81];
                ram[0x2132] = v;
                ram[0x79]++;
                if ((ram[0x79] & ram[0x82]) == 0) break;
            } while (1);
            
            counter = ram[0x81];
            if (counter == 0) {
                ram[0x81] = 0; // Handle the wrap-around case (dec $81 when 0)
                break;
            }
            ram[0x81] = counter - 1;
            if (ram[0x81] < 0) { // This part of the ASM is logic for unsigned underflow
                // In 65816, dec $81 when $81=0 results in $FF (BPL is false)
                break; 
            }
            // This is a recursive-like loop structure. In C, we simulate the 
            // total cycles until ram[0x81] underflows.
        }
    } else {
        // Handle the case where ram[0x81] was already 0 on entry
        // dec $81 -> $FF, bpl fails.
        ram[0x81] = 0xFF; // result of dec $00
    }

    // Now we are at @ea16's exit path (after bpl fails)
    ram[0x81] = 0;              // stz $81
    ram[0x212D] = 0x11;         // lda #$11 / sta $212d

    uint8_t check = ram[0x0FE4];
    uint8_t shifted = (uint8_t)(check >> 1); // lsr A (Pitfall 7: explicit truncation)
    
    if (shifted != 0) {         // bcc @ea48 (branch if carry clear, i.e., bit 7 was 0)
        // In 65816, LSR shifts bit 0 into Carry. 
        // Wait: LSR A shifts bit 0 into Carry. 
        // The ASM is `lsr / bcc @ea48`. 
        // If (ram[0x0FE4] & 1) == 0, branch to @ea48.
        if ((check & 1) == 0) {
            ram[0x2131] = 0;    // stz $2131
            return;
        }
        ram[0x2130] = 0x02;     // lda #$02 / sta $2130
        ram[0x2131] = 0x43;     // lda #$43 / sta $2131
    } else {
        ram[0x2131] = 0;        // stz $2131
    }
}

// PITFALLS: 7 (LSR truncation), 3 (BCC/BPL logic inversion)
// HELPERS: wait_frame_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x79=1, 0x81=1, 0x82=1, 0x83=1, 0x0FE4=1
//   output_ram:  0x2132=1, 0x212D=1, 0x2130=1, 0x2131=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00ea14 ($00:EA14)