#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0xFD, DP=0
// This routine processes controller input for menu navigation.
// It handles multi-controller selection in battle vs combined input in normal mode.
// 
// Logic:
// 1. If multi-controller flag AND in-battle flag are set:
//    - Fetch current character, find their assigned controller (0 or 2).
//    - Read only that controller's input.
// 2. Otherwise:
//    - OR the input from controller 0 and controller 2.
// 3. Handle input repeat counters:
//    - If input changed: reset counter to 0x18, update current/previous state.
//    - If input held: decrement counter; if it hits 0, trigger repeat at 0x03.
static void ReadCtrl_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    uint8_t a;
    uint8_t ctrl_val;

    // Check multi-controller flag and in-battle flag
    if ((ram[0x16B8] & 0x40) != 0) {
        //- Battle multi-controller logic
        uint8_t selected_char = ram[0x001822]; // ROM/WRAM address f:001822
        ram[0x43] = selected_char;
        
        // y = selected_char
        // lda $16B9, y -> controller mapping (0 or 2)
        uint8_t controller_idx = ram[0x16B9 + selected_char];
        
        // 16-bit math for pointer calculation (longa)
        // A = selected_char + controller_idx
        uint16_t offset = (uint16_t)selected_char + (uint16_t)controller_idx;
        
        // lda a:$0000, y -> Read input from the calculated offset
        // Since DP is 0, this reads from ram[offset]
        ctrl_val = ram[offset];
    } else {
        //- Combined input logic (non-battle or single controller)
        // lda a:$0000, y / ora a:$0002, y
        // Note: Y is inherited from caller or previous state. 
        // In this path, Y is used as the base index for the controller input area.
        ctrl_val = ram[cpu->y] | ram[cpu->y + 2];
    }

    if (ctrl_val == 0) { // beq @fdc0
        // No buttons pressed: clear input and reset counter
        ram[0x04 + cpu->x] = ctrl_val; // sta $04,x
        ram[0x00 + cpu->x] = 0;        // sta $00,x
        ram[0x08 + cpu->x] = 0x18;     // sta $08,x
        return;
    }

    if (ctrl_val == ram[0x04 + cpu->x]) { // cmp $04,x / beq @fdc9
        // Buttons held (unchanged)
        uint8_t repeat_cnt = ram[0x08 + cpu->x];
        if (repeat_cnt > 0) {
            ram[0x08 + cpu->x] = (uint8_t)(repeat_cnt - 1); // dec $08,x
            if (repeat_cnt != 1) { // beq @fdd0 (branch if result was 0)
                ram[0x00 + cpu->x] = 0; // stz $00,x
                return;
            }
        }
        // Repeat trigger reached (@fdd0)
        ram[0x08 + cpu->x] = 0x03; 
        ram[0x00 + cpu->x] = ram[0x04 + cpu->x];
        return;
    }

    // Input changed: update state and set first repeat counter
    ram[0x04 + cpu->x] = ctrl_val;
    ram[0x00 + cpu->x] = ctrl_val;
    ram[0x08 + cpu->x] = 0x18;
}

// PITFALLS: 6 (Mode A transition: longa/shorta used to compute controller offset),
//            7 (8-bit truncation on dec $08,x)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=bits(8), y=bits(8)
//   inputs_ram:  0x16B8=1, 0x16B9=1, 0x001822=1, 0x04+X=1, 0x08+X=1
//   output_ram:  0x00+X=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFD
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: menu::ReadCtrl ($FD:90)