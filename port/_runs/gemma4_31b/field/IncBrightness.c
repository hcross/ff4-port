#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x00, DP=0
// Logic:
//   If (ram[0x7A] & 0x07) != 0, skip increment.
//   If (ram[0x80] == 0x0F), skip increment.
//   Otherwise, increment brightness value at 0x80.
//   Finally, write current brightness (0x80) to hardware register 0x2100.
static void IncBrightness_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // lda $7a / and #$07 / bne @dbcc
    if ((ram[0x7A] & 0x07) == 0) {
        // lda $80 / cmp #$0f / beq @dbcc
        if (ram[0x80] != 0x0F) {
            ram[0x80]++; // inc $80
        }
    }

    // @dbcc: lda $80 / sta $2100
    // Note: $2100 is outside WRAM, but based on snesrev/zelda3 pattern,
    // we write to the mapped hardware/IO space in snes->ram if provided 
    // or a specific IO register. In this context, we treat the destination 
    // as an absolute write.
    snes->ram[0x2100] = ram[0x80];
}

// PITFALLS: 7 (8-bit truncation for INC, though not overflowing 0x0F here)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x7A=1, 0x80=1
//   output_ram:  0x2100=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::IncBrightness ($00:00BE)