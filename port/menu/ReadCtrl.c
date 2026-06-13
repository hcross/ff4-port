#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00 (code bank), DP=0
// Logic:
//   Reads controller input based on whether multi-controller mode and battle mode are active.
//   If both are active: fetches controller mapping for the selected character.
//   Otherwise: ORs inputs from controllers 0 and 2.
//   Handles input debouncing/repeat counters via ram[$04,x] and ram[$08,x].
static void ReadCtrl_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // lda $16b8 / and $40
    uint8_t multi_ctrl = ram[0x16B8];
    uint8_t in_battle = multi_ctrl & 0x40;

    uint8_t input;
    if (in_battle != 0) { // beq @fdb4 (inverted: if flags set, execute block)
        // lda f:$001822 (Bank 0:001822)
        uint8_t selected_char = ram[0x1822];
        ram[0x43] = selected_char; // sta $43

        // ldy $43 / lda $16b9,y
        uint8_t ctrl_idx = ram[0x16B9 + selected_char];
        ram[0x43] = ctrl_idx; // sta $43 (after ply)

        // longa / tya / clc / adc $43 / tay / shorta
        // Calculation: Y = selected_char + ctrl_idx (controller offset)
        uint16_t y_offset = (uint16_t)selected_char + (uint16_t)ctrl_idx;
        
        // lda a:$0000,y (Bank 0:0000 + y_offset)
        input = ram[y_offset];
    } else {
        // @fdb4: lda a:$0000,y / ora a:$0002,y
        // Y is inherited from caller (X/Y are 16-bit)
        input = ram[cpu->y] | ram[cpu->y + 2];
    }

    // @fdba: beq @fdc0
    if (input == 0) {
        // @fdc0:
        ram[0x04 + cpu->x] = input; // sta $04,x
        ram[0x00 + cpu->x] = input; // sta $00,x
        ram[0x08 + cpu->x] = 0x18;   // sta $08,x
        return;
    }

    // cmp $04,x / beq @fdc9
    if (input == ram[0x04 + cpu->x]) {
        // @fdc9: dec $08,x
        uint8_t repeat = ram[0x08 + cpu->x] - 1; // Pitfall 7: 8-bit truncation
        ram[0x08 + cpu->x] = repeat;

        if (repeat == 0) { // beq @fdd0
            // @fdd0:
            ram[0x08 + cpu->x] = 0x03; // sta $08,x
            ram[0x00 + cpu->x] = ram[0x04 + cpu->x]; // sta $00,x
        } else {
            ram[0x00 + cpu->x] = 0; // stz $00,x
        }
        return;
    }

    // @fdc0:
    ram[0x04 + cpu->x] = input; // sta $04,x
    ram[0x00 + cpu->x] = input; // sta $00,x
    ram[0x08 + cpu->x] = 0x18;   // sta $08,x
}

// PITFALLS: 6 (Mode A explicit shift between 8/16 bit), 7 (8-bit decrement truncation),
// 8 (Inherited X/Y 16-bit mode).
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=16
//   inputs_ram:  0x16B8=1, 0x1822=1, 0x16B9=1, 0x0000=1, 0x0400=1, 0x0800=1
//   output_ram:  0x0000=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: menu::ReadCtrl ($FD:90)