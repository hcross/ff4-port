#include "snes/snes.h"

// Entry mode: A 16-bit (mf=0), X 16-bit (xf=0), DB=0x7E, DP=0
// Entry: cpu->a = 16-bit input value (treated as signed)
// Output: result in $1e (16-bit), carry set if result was negative and saturated
static void _13e58b_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    uint16_t a = cpu->a & 0x01FF;        // and #$01ff
    a = (uint16_t)(a << 1);              // asl (16-bit)
    cpu->x = a;                          // tax

    // Accessing f:SolarSystemSineTbl,x (bank F5)
    uint16_t sine_val = read16(ram, 0xF50000 + a);
    write16(ram, 0x1A, sine_val);

    if ((sine_val & 0x8000) != 0) {      // bpl @e5b1 (checking sign bit)
        // Negative branch
        sine_val ^= 0xFFFF;              // eor #$ffff
        write16(ram, 0x1A, sine_val);    // sta $1a

        Mult16_2_emu(snes);              // jsr Mult16_2

        uint16_t result = read16(ram, 0x1E);
        result ^= 0xFFFF;                // eor #$ffff
        result++;                        // inc

        if ((result & 0x8000) != 0) {    // bpl @e5be (checking sign bit)
            write16(ram, 0x1E, result);  // sta $1e
            cpu->mf = true;              // shorta0
            cpu->c = true;               // sec
            return;
        }
        write16(ram, 0x1E, result);      // sta $1e
        cpu->mf = true;                  // shorta0
        cpu->c = false;                  // clc
        return;
    }

    // Positive branch
    write16(ram, 0x1A, sine_val);        // sta $1a

    Mult16_2_emu(snes);                  // jsr Mult16_2

    uint16_t result = read16(ram, 0x1E);
    if ((result & 0x8000) != 0) {        // bmi @e5aa
        write16(ram, 0x1E, result);      // sta $1e
        cpu->mf = true;                  // shorta0
        cpu->c = true;                   // sec
        return;
    }

    write16(ram, 0x1E, result);          // sta $1e
    cpu->mf = true;                      // shorta0
    cpu->c = false;                      // clc
}

// PITFALLS: 1 (DB=0x7E assumed for WRAM access), 6 (Mode A 16-bit),
//           8 (Mode A/X heritage - routine starts with longa, so inherits mf=0)
// HELPERS: Mult16_2_emu(snes), read16/write16
// CONTRACT:
//   inputs_reg:  a=16, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x1e=2
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::_13e58b ($E5:8B)