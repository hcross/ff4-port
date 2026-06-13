#include "snes/snes.h"

// Entry mode: A 16-bit (mf=0), X 16-bit (xf=0), DB=$7E, DP=0
// Entry: cpu->a = 16-bit input value (treated as signed)
// Output: result in $14 (16-bit), carry set or clear on return
static void _13ec24_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    uint16_t a = cpu->a & 0x01FF;        // and #$01ff
    a <<= 1;                             // asl (16-bit)
    cpu->x = a;                          // tax

    // Access SolarSystemSineTbl in bank EC
    cpu->db = 0xEC;
    uint16_t sine_val = read16(&snes->cart->rom[0xEC * 0x8000], a); // f:SolarSystemSineTbl,x

    if ((sine_val & 0x8000) == 0) {      // bpl @ec4a
        write16(ram, 0x10, sine_val);    // sta $10
        Mult16_1_emu(snes);              // jsr Mult16_1
        uint16_t result = read16(ram, 0x14); // lda $14
        if ((result & 0x8000) != 0) {    // bmi @ec43
            write16(ram, 0x14, result);
            cpu->mf = true;              // shorta0
            cpu->c = true;               // sec
            return;
        }
        write16(ram, 0x14, result);
        cpu->mf = true;                  // shorta0
        cpu->c = false;                  // clc
        return;
    }

    // Negative sine branch
    sine_val ^= 0xFFFF;                  // eor #$ffff
    write16(ram, 0x10, sine_val);        // sta $10
    Mult16_1_emu(snes);                  // jsr Mult16_1
    uint16_t result = read16(ram, 0x14); // lda $14
    result ^= 0xFFFF;                    // eor #$ffff
    result++;                            // inc
    if ((result & 0x8000) == 0) {        // bpl @ec57
        write16(ram, 0x14, result);
        cpu->mf = true;                  // shorta0
        cpu->c = false;                  // clc
        return;
    }
    write16(ram, 0x14, result);
    cpu->mf = true;                      // shorta0
    cpu->c = true;                       // sec
}

// PITFALLS: 1 (DB must be set to 0xEC for ROM access), 6 (mode A is 16-bit),
// 8 (inherited mode A/X from caller)
// HELPERS: Mult16_1_emu(snes) — delegates Mult16_1 @ $00:E512
// CONTRACT:
//   inputs_reg:  a=16, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x14=2
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::_13ec24 ($EC:24)