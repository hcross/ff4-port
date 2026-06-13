#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// No input registers; all data read from/written to WRAM
// This function computes moon position and draws the moon sprite
static void UpdateMoon2_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // ldx #$0002 / jsr GetMoonTile
    cpu->x = 0x0002;
    get_moon_tile_emu(snes);

    // longa
    cpu->mf = false;

    // lda $78 / clc / adc $6c / adc $7c / adc #$0008 / sta $4b
    uint16_t a = read16(ram, 0x78);
    a += read16(ram, 0x6C);
    a += read16(ram, 0x7C);
    a += 0x0008;
    write16(ram, 0x4B, a);

    // lda $7a / clc / adc $6e / adc $7e / adc #$0008 / sta $4d
    a = read16(ram, 0x7A);
    a += read16(ram, 0x6E);
    a += read16(ram, 0x7E);
    a += 0x0008;
    write16(ram, 0x4D, a);

    // shorta0
    cpu->mf = true;

    // lda #$04 / sta $51
    ram[0x51] = 0x04;

    // jsr DrawSolarSystemSprite
    draw_solar_system_sprite_emu(snes);
}

// PITFALLS: 1 (DB=$7E required for WRAM access), 6 (mode A 16-bit during math),
//           8 (inherited mode A 8-bit, X/Y 16-bit)
// HELPERS: get_moon_tile_emu(snes), draw_solar_system_sprite_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x6C=2, 0x6E=2, 0x78=2, 0x7A=2, 0x7C=2, 0x7E=2
//   output_ram:  0x4B=2, 0x4D=2, 0x51=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::UpdateMoon2 ($E2:81)