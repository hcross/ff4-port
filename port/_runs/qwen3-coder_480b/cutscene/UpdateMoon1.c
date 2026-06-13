#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// All inputs/outputs in WRAM (no register I/O — convention cutscene):
//   in : $6C-$6F, $70-$73, $74-$77, $7C-$7F, $88-$8B, $A1-$A4
//   out: $4B-$4E, $51, $5D-$5E, $5F-$60, $20C0-$20C1, $0061
static void UpdateMoon1_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    get_moon_tile_emu(snes);  // ldx #$0000 / jsr GetMoonTile

    // longa
    uint16_t a = read16(ram, 0x74);
    a = (uint16_t)(a + read16(ram, 0x6C));
    a = (uint16_t)(a + read16(ram, 0x7C));
    a = (uint16_t)(a + 0x0008);
    write16(ram, 0x4B, a);  // sta $4b

    a = read16(ram, 0x76);
    a = (uint16_t)(a + read16(ram, 0x6E));
    a = (uint16_t)(a + read16(ram, 0x7E));
    a = (uint16_t)(a + 0x0008);
    write16(ram, 0x4D, a);  // sta $4d

    // shorta0
    ram[0x51] = 0x04;  // sta $51

    if (ram[0x5B] == 0) {  // lda a:$005b / beq @e243
        draw_solar_system_sprite_emu(snes);  // jsr DrawSolarSystemSprite
        return;
    }

    // longa
    a = read16(ram, 0x70);
    a = (uint16_t)(a + read16(ram, 0x88));
    a ^= 0xFFFF;  // eor #$ffff
    a = (uint16_t)(a - read16(ram, 0xA1));  // sec / sbc $a1
    write16(ram, 0x5D, a);  // sta $5d

    a = read16(ram, 0x72);
    a = (uint16_t)(a + read16(ram, 0x8A));
    a ^= 0xFFFF;  // eor #$ffff
    a = (uint16_t)(a - read16(ram, 0xA3));  // sec / sbc $a3
    write16(ram, 0x5F, a);  // sta $5f

    // shorta0
    uint16_t x = read16(ram, 0x61);
    if (x >= 0x0004) {  // cpx #$0004 / bcc @e242
        x -= 3;         // dex3
        write16(ram, 0x61, x);
        x = read16(ram, 0x20C0);
        x += 2;         // inx2
        write16(ram, 0x20C0, x);
    }
}

// PITFALLS: 1 (DB=$7E required for absolute stores), 6 (mode A 8/16-bit),
// 7 (arithmetic truncation in 8-bit mode), 8 (mode A/X heritage)
// HELPERS: get_moon_tile_emu(snes), draw_solar_system_sprite_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x6C=2, 0x6E=2, 0x70=2, 0x72=2, 0x74=2, 0x76=2,
//                0x7C=2, 0x7E=2, 0x88=2, 0x8A=2, 0xA1=2, 0xA3=2
//   output_ram:  0x4B=2, 0x4D=2, 0x51=1, 0x5D=2, 0x5F=2, 0x20C0=2, 0x0061=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::UpdateMoon1 ($E1:E1)