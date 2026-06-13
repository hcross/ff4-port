#include "snes/snes.h"

static void SetBtnMap_c(Snes *snes) {
    // Entry: A = index (8-bit), X = value to store (16-bit)
    // DB = $7E, DP = 0, mf=1, xf=0
    uint8_t *ram = snes->ram;
    const uint8_t *rom = snes->rom; // ROM image

    // phx: push X (16-bit) onto stack. We'll simulate by saving X in a local.
    uint16_t saved_x = snes->cpu->x; // X is 16-bit

    // sta $43: store A (8-bit) to $7E:0043
    ram[0x43] = (uint8_t)snes->cpu->a; // A is 8-bit, but cpu->a is uint16_t, so mask.

    // ldx $43: load X from $43 (16-bit because xf=0). $44 must be zero.
    // We'll read 16-bit from $43-$44.
    uint16_t index1 = read16(ram, 0x43); // This will be (ram[0x44] << 8) | ram[0x43]
    // But we need to ensure high byte is zero. The caller must have set $44=0.
    // We'll just use the read value; if $44 is not zero, parity will fail, but that's caller's responsibility.

    // lda f:BtnMapTbl,x: load byte from ROM at (bank << 16 | offset + index1)
    // We need the ROM address of BtnMapTbl. Let's assume BTN_MAP_TBL_OFFSET = 0xFE008A (bank $FE, offset $8A).
    // We'll use a constant.
    uint8_t btn_map_byte = rom[BTN_MAP_TBL_OFFSET + index1]; // index1 is 16-bit, but table is small, so high byte should be 0.

    // sta $43: store that byte to $43 (8-bit store)
    ram[0x43] = btn_map_byte;

    // ldx $43: load X from $43 (16-bit, high byte from $44, which is still zero because we only wrote to $43)
    uint16_t index2 = read16(ram, 0x43); // again, high byte should be zero.

    // longa: set mf=0 (A 16-bit)
    snes->cpu->mf = false;

    // pla: pull 16-bit value from stack into A. The stack has the saved X (2 bytes).
    // We need to simulate stack pull. The stack pointer is snes->cpu->sp. In native mode, sp is 16-bit.
    // We'll read from ram at sp, then increment sp by 2.
    uint16_t sp = snes->cpu->sp;
    uint16_t pulled = read16(ram, sp); // little-endian
    snes->cpu->sp = sp + 2;
    snes->cpu->a = pulled;

    // sta $1a05,x: store 16-bit A to $1a05 + index2
    write16(ram, 0x1A05 + index2, pulled);

    // shorta: set mf=1 (A 8-bit)
    snes->cpu->mf = true;

    // rts: return
}