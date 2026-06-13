#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// All inputs/outputs in WRAM or hardware registers (no register I/O):
//   in : ram[$1704] = area ID, ram[$7A] = animation frame
//   out: hardware palette $0EC7-$0EC8 = 16-bit little-endian color word
static void UpdateWhalePal_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    if (ram[0x1704] != 6) return;         // cmp #$06 / bne @c3d9
    uint8_t frame = ram[0x7A];            // lda $7a
    uint8_t index = (frame >> 2) & 0x0E;  // lsr2 / and #$0e
    // Read 16-bit color from WhalePal LUT (ROM table, 16-bit entries)
    uint16_t color = read16(WhalePal, index);
    write16(ram, 0x0EC7, color);          // store to hardware palette
}

// PITFALLS: 1 (DB assumed $7E by default, but this writes to hardware
// palette, so no issue), 6 (A 8-bit mode assumed), 7 (no arithmetic
// truncation needed), 8 (no mode change — caller handles)
// HELPERS: read16 (for 16-bit LUT access), write16 (for palette output)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1704=1, 0x7A=1
//   output_ram:  0x0EC7=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::UpdateWhalePal ($C3:BD)