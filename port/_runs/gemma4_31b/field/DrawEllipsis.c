#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$B4 (implied by ROM), DP=0
// Logic:
//   Saves the current A value to specific WRAM offsets (shifted by Y),
//   writes 0xFF to two other sets of offsets,
//   increments the pointer at $3D by 2, and jumps to _b4b9.
static void DrawEllipsis_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = (uint8_t)snes->cpu->a;
    uint16_t y = snes->cpu->y;

    // xba / lda #$ff / xba
    // The sequence effectively stores the original A into X, loads 0xFF,
    // then restores original A into X. However, in C, we just need
    // to track that $0774/0775 get 0xFF and $0834/0835 get the original A.
    
    ram[0x0774 + y] = 0xFF; // sta $0774,y (after lda #$ff)
    ram[0x0775 + y] = 0xFF; // sta $0775,y

    ram[0x0834 + y] = a;    // sta $0834,y (after xba restored original A)
    ram[0x0835 + y] = a;    // sta $0835,y

    // lda #0 / xba / ldy $3d / iny2 / sty $3d
    // The lda #0 / xba sequence is effectively a no-op for the pointer logic
    // because A is not used after the xba.
    uint16_t ptr = read16(ram, 0x3D);
    ptr += 2; // iny2
    write16(ram, 0x3D, ptr);

    // jmp _b4b9
    _b4b9_emu(snes);
}

// PITFALLS: None applicable (direct memory writes and simple pointer arithmetic)
// HELPERS: _b4b9_emu(snes) — delegates jump target _b4b9
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=16
//   inputs_ram:  0x3D=2
//   output_ram:  0x0774=<1>, 0x0775=<1>, 0x0834=<1>, 0x0835=<1>, 0x3D=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB4
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::DrawEllipsis ($B4:FF)