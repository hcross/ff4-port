#include "snes/snes.h"

// This function performs a nested loop to process tilemap data,
// calling a helper to prepare each tile and then transferring
// data from a source address to a destination in VRAM.
// It uses 16-bit indices and pointers, with 8-bit accumulator
// for data transfer, switching to 16-bit mode temporarily for
// address arithmetic.
static void _13d92d_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    write16(ram, 0x66, 0xFE00);
    write16(ram, 0x53, 0x8000);
    ram[0x55] = 0x7E;

    for (uint16_t x = 0; x < 0x20; x++) {
        _13d990_emu(snes);
        write16(ram, 0x00, 0x2F03);
        // clr_ax → tdc / tax (A = DP = 0, X = 0)
        write16(ram, 0x0A, 0);

        for (uint8_t row = 0; row < 0x20; row++) {
            ram[0x08] = 0x20;
            uint16_t y = read16(ram, 0x0A);

            for (uint8_t i = 0; i < 0x20; i++) {
                // lda ($00),y
                uint16_t src = read16(ram, 0x00);
                uint8_t data = ram[src + y];
                // sta [$53]
                uint16_t dst = read16(ram, 0x53);
                ram[dst] = data;

                // longa
                // inc $53
                write16(ram, 0x53, (uint16_t)(dst + 1));

                // tya / clc / adc #$0020 / tay
                y = (uint16_t)(y + 0x20);
                // shorta0
            }

            // inc $0a
            write16(ram, 0x0A, (uint16_t)(read16(ram, 0x0A) + 1));
        }
    }
}

// PITFALLS: 1 (DB must be $7E for writes to $53/$55), 6 (A mode switching),
// 7 (16-bit arithmetic truncation), 8 (inherited A/X mode), 10 (goto label
// declarations — not used here)
// HELPERS: _13d990_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::_13d92d ($D9:2D)