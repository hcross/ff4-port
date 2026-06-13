#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$DE, DP=0
// This routine prepares a coordinate-based context in the Direct Page (WRAM)
// for field processing, then invokes two specific sub-routines.
static void _00de43_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // pha: preserve A (8-bit)
    uint8_t saved_a = (uint8_t)cpu->a;

    // ldy #$01d0
    cpu->y = 0x01D0;

    // lda #$1c / sta $91
    ram[0x91] = 0x1C;

    // lda #$78 / sta $8f
    ram[0x8F] = 0x78;

    // ldx $2c / stx $0c (X is 16-bit)
    uint16_t x_val_2c = read16(ram, 0x2C);
    write16(ram, 0x0C, x_val_2c);

    // ldx $2e / stx $0e
    uint16_t x_val_2e = read16(ram, 0x2E);
    write16(ram, 0x0E, x_val_2e);

    // lda #$20 / sta $ad
    ram[0xAD] = 0x20;

    // pla / sta $92
    ram[0x92] = saved_a;

    // jsr _00dfc4
    _00dfc4_emu(snes);

    // ldy #$0190
    cpu->y = 0x0190;

    // jsr _00e013
    _00e013_emu(snes);
}

// PITFALLS: 1 (DB=$DE), 6 (Mode A 8-bit/Mode X 16-bit mix)
// HELPERS: _00dfc4_emu(snes), _00e013_emu(snes)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x2C=2, 0x2E=2
//   output_ram:  0x91=1, 0x8F=1, 0x0C=2, 0x0E=2, 0xAD=1, 0x92=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xDE
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00de43 ($DE:43)