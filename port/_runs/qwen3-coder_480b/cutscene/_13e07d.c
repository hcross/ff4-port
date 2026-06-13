#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// Entry: ram[$4A] is input (tested for low nybble zero)
// Logic:
//   if ((ram[$4A] & 0x0F) != 0) return;
//   else:
//     1. set $28 = 0x0400
//     2. call _13e058(0x7E, $57, 0x4000, 0x0400)
//     3. update $57 = (($57 + 0x0400) & 0x7FFF) | 0x8000
static void _13e07d_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = ram[0x4A];
    if ((a & 0x0F) != 0) {         // and #$0f / bne @e0a5
        return;
    }
    write16(ram, 0x28, 0x0400);    // ldx #$0400 / stx $28
    uint16_t x = read16(ram, 0x57); // ldx $57
    uint16_t y = 0x4000;           // ldy #$4000
    uint8_t a8 = 0x7E;             // lda #$7e
    // Call _13e058 with A=0x7E, X=$57, Y=0x4000, param=0x0400
    Cpu *cpu = snes->cpu;
    cpu->a = a8;
    cpu->x = x;
    cpu->y = y;
    cpu->db = 0x7E;
    cpu->dp = 0;
    cpu->mf = true;
    cpu->xf = false;
    _13e058_emu(snes);             // jsr _13e058 (delegated)

    // longa
    cpu->mf = false;
    uint16_t val = read16(ram, 0x57); // lda $57
    val = (uint16_t)(val + 0x0400);   // clc / adc #$0400
    val &= 0x7FFF;                    // and #$7fff
    val |= 0x8000;                    // ora #$8000
    write16(ram, 0x57, val);          // sta $57
    // shorta0
    cpu->mf = true;
    cpu->a = 0;
}

// PITFALLS: 1 (DB must be $7E for JSR target), 6 (A mode changes),
//           8 (A/X mode inherited — assumed A=8, X=16 from caller)
// HELPERS: _13e058_emu(snes) — delegates _13e058 @ E0:58
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x4A=1, 0x57=2
//   output_ram:  0x28=2, 0x57=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::_13e07d ($E0:7D)