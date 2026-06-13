#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: cpu->a = input value (8-bit)
// Logic:
//   1. Split input into bitfields:
//        $06 = (input & 0xE0)
//        $83 = ((input & 0x0F) << 1) + (input & 0xE0) + 1
//        $82 = (input & 0x10) ? 7 : 0
//   2. Loop calling WaitFrame, setting color math registers,
//      incrementing $79 until ($79 & $82) != 0
//   3. Then increment $81, compare with ($83 & 0x1F), loop back
//      if not equal. Finally decrement $81 and return.
static void _00e9cf_c(Snes *snes, uint8_t input) {
    uint8_t *ram = snes->ram;
    uint8_t temp = input & 0xE0;
    ram[0x06] = temp;
    uint8_t shifted = (input & 0x0F) << 1;
    uint8_t sum = (uint8_t)(shifted + temp + 1); // carry handled by adc in asm
    ram[0x83] = sum;
    if (input & 0x10) {
        ram[0x82] = 7;
    } else {
        ram[0x82] = 0;
    }
    ram[0x79] = 0;
    ram[0x81] = 0;

loop_e9eb:;
    wait_frame_emu(snes);
    ram[0x212D] = 0;
    ram[0x2131] = 0x83;
    uint8_t color = (ram[0x83] & 0xE0) | ram[0x81];
    ram[0x2132] = color;
    ram[0x79]++;
    if ((ram[0x79] & ram[0x82]) != 0) goto loop_e9eb; // bne @e9eb
    ram[0x81]++;
    if ((ram[0x83] & 0x1F) != ram[0x81]) goto loop_e9eb; // bne @e9eb
    ram[0x81]--;
}

// PITFALLS: 1 (DB=$7E), 6 (mode A 8-bit), 7 (arithmetic truncation)
// HELPERS: wait_frame_emu(snes)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00e9cf ($E9:CF)