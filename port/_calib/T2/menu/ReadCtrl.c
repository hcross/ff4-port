#include "snes/snes.h"

// Reads controller input for a character in battle.
// If in battle and character has a controller, uses that.
// Otherwise combines input from both controllers.
// Handles input repeat timing.
static void ReadCtrl_c(Snes *snes, uint16_t x, uint16_t y) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    uint8_t ctrl_flag = ram[0x16B8];
    uint8_t battle_flag = ram[0x40];
    if ((ctrl_flag & battle_flag) == 0) {  // beq @fdb4
        goto combine_input;
    }

    uint8_t char_index = ram[0x1822];      // selected character
    ram[0x43] = char_index;

    uint16_t y_save = cpu->y;              // phy
    cpu->y = char_index;
    uint8_t controller = ram[0x16B9 + cpu->y];  // 0 or 2
    cpu->y = y_save;                       // ply
    ram[0x43] = controller;

    cpu->mf = false;                       // longa
    uint16_t sum = (uint16_t)(cpu->y + controller);  // tya; clc; adc $43; tay
    cpu->mf = true;                        // shorta
    cpu->y = sum;

    cpu->a = ram[cpu->y];                  // lda a:$0000,y
    goto check_input;

combine_input:
    cpu->a = ram[cpu->y] | ram[cpu->y + 2];  // ora a:$0002,y

check_input:;                              // @fdba
    uint8_t input = (uint8_t)cpu->a;
    if (input == 0) {                      // beq @fdc0
        goto store_input;
    }
    if (input == ram[0x04 + x]) {          // cmp $04,x; beq @fdc9
        goto input_held;
    }

store_input:                               // @fdc0
    ram[0x04 + x] = input;
    ram[0x00 + x] = input;
    ram[0x08 + x] = 0x18;                  // set first repeat counter
    return;

input_held:                                // @fdc9
    ram[0x08 + x]--;                       // dec $08,x
    if (ram[0x08 + x] == 0) {              // beq @fdd0
        goto repeat_input;
    }
    ram[0x00 + x] = 0;                     // stz $00,x
    return;

repeat_input:                              // @fdd0
    ram[0x08 + x] = 0x03;                  // set multi-repeat counter
    ram[0x00 + x] = ram[0x04 + x];         // repeat the input
    return;
}

// PITFALLS: 6 (mode A switching: longa/shorta), 7 (16-bit arithmetic truncation)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=16
//   inputs_ram:  0x16B8=1, 0x40=1, 0x1822=1, 0x16B9=1, 0x00=1, 0x02=1, 0x04=1, 0x08=1
//   output_ram:  0x00=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: menu::ReadCtrl ($FD:90)