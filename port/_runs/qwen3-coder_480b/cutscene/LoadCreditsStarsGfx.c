#include "snes/snes.h"

// LoadCreditsStarsGfx: Transfers 3bpp graphics for the credits starfield.
// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// No input registers; all parameters are compile-time constants.
// No output registers; writes to VRAM via Tfr3bppGfx.
static void LoadCreditsStarsGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // ldx #$0032 / stx $00
    cpu->x = 0x0032;
    write16(ram, 0x00, cpu->x);

    // ldx #.loword(CreditsStarsGfx) / ldy #$4000 / lda #^CreditsStarsGfx
    cpu->x = 0xE992; // .loword(CreditsStarsGfx)
    cpu->y = 0x4000;
    cpu->a = 0xD8;   // ^CreditsStarsGfx

    // jsr Tfr3bppGfx
    tfr3bpp_gfx_emu(snes);
}

// PITFALLS: 1 (DB=$7E required for stx $00), 8 (A 8-bit, X/Y 16-bit assumed)
// HELPERS: tfr3bpp_gfx_emu(snes) — delegates Tfr3bppGfx @ $00:860F
// CONTRACT:
//   inputs_reg:  a=8, x=16, y=16
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::LoadCreditsStarsGfx ($D8:E9)