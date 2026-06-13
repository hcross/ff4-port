#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0, DP=0
// This routine checks the current hardware joystick state ($02 and $03)
// against specific button masks. If a button is NOT currently pressed
// (the bit is 0), it clears the corresponding soft-state button flag in RAM.
static void ResetButtons_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // A button: check bit, clear $54 if not pressed
    if (!(ram[0x02] & JOY_A)) {
        ram[0x54] = 0;
    }

    // X button: check bit, clear $50 if not pressed
    if (!(ram[0x02] & JOY_X)) {
        ram[0x50] = 0;
    }

    // L button: check bit, clear $52 if not pressed
    if (!(ram[0x02] & JOY_L)) {
        ram[0x52] = 0;
    }

    // R button: check bit, clear $53 if not pressed
    if (!(ram[0x02] & JOY_R)) {
        ram[0x53] = 0;
    }

    // B button: check bit, clear $55 if not pressed
    if (!(ram[0x03] & JOY_B)) {
        ram[0x55] = 0;
    }

    // Y button: check bit, clear $51 if not pressed
    if (!(ram[0x03] & JOY_Y)) {
        ram[0x51] = 0;
    }

    // Select button: check bit, clear $56 if not pressed
    if (!(ram[0x03] & JOY_SELECT)) {
        ram[0x56] = 0;
    }

    // Start button: check bit, clear $57 if not pressed
    if (!(ram[0x03] & JOY_START)) {
        ram[0x57] = 0;
    }
}

// PITFALLS: None. Direct 8-bit memory access and conditional checks.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x02=1, 0x03=1
//   output_ram: none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (multiple output RAM addresses)

// REVERSED_FUNCTION: field::ResetButtons ($CA:1D)