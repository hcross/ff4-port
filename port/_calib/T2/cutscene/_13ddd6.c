#include "snes/snes.h"

// Entry mode: A 16-bit (mf=0), X 16-bit (xf=0), DB=0x7E, DP=0
// Entry: A = value for $4354, X = value for $4352, Y = value for hVMADDL
// This function configures DMA channel 5 for a transfer to VRAM.
static void _13ddd6_c(Snes *snes, uint16_t a, uint16_t x, uint16_t y) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // Simulate PHB PHA CLR_A PHA PLB PLA (stack and DB setup)
    // These are not relevant to observable state, so omitted

    // Entry point state (Y, X, A already provided as args)
    write16(ram, 0x2116, y);         // sty hVMADDL ($2116–$2117)
    write16(ram, 0x4352, x);         // stx $4352
    write16(ram, 0x4354, a);         // sta $4354

    ram[0x4350] = 0x01;              // lda #$01 / sta $4350
    ram[0x4351] = 0x18;              // lda #<hVMDATAL ($2118) / sta $4351

    write16(ram, 0x4355, 0x0000);    // ldx $00 / stx $4355 (X=0 from caller)

    ram[0x420B] = 0x20;              // lda #$20 / sta hMDMAEN ($420B)

    // PLB RTS simulated by return
}

// PITFALLS: 1 (DB must be $7E for WRAM access, but this routine writes
// to hardware registers — no issue), 4 (stack addr depends on E flag,
// but this routine doesn't access stack after PLB)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=16, x=16, y=16
//   inputs_ram:  none
//   output_ram:  0x2116=2, 0x4350=1, 0x4351=1, 0x4352=2, 0x4354=2, 0x4355=2, 0x420B=1
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::_13ddd6 ($DD:D6)