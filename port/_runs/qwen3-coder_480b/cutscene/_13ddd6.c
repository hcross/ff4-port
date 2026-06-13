#include "snes/snes.h"

// This function sets up MDMA transfer to VRAM, using registers X and Y as
// source/destination addresses and A as the transfer size.
// Entry mode: A 16-bit (mf=0), X 16-bit (xf=0), Y 16-bit (from longi context)
// DB is pushed and set to 0, then restored (phb/plb)
static void _13ddd6_c(Snes *snes, uint16_t a, uint16_t x, uint16_t y) {
    uint8_t *ram = snes->ram;
    // phb / pha / clr_a / pha / plb
    // Effect: DB = 0 for the duration of the routine
    //         A is cleared (but not used further)
    // sty hVMADDL (Y -> VRAM address)
    write16(ram, 0x2116, y);  // hVMADDL is at $2116-$2117
    // stx $4352 (X -> DMA source offset)
    write16(ram, 0x4352, x);
    // sta $4354 (A -> DMA size)
    write16(ram, 0x4354, a);
    // lda #$01 / sta $4350 (DMA control: direction + fixed source)
    ram[0x4350] = 0x01;
    // lda #<hVMDATAL / sta $4351 (DMA dest: VRAM data low)
    ram[0x4351] = 0x18;  // hVMDATAL = $2118
    // ldx $00 / stx $4355 (DMA source bank = 0)
    write16(ram, 0x4355, read16(ram, 0x00));
    // lda #$20 / sta hMDMAEN (Trigger DMA channel 5)
    ram[0x420B] = 0x20;  // hMDMAEN is at $420B
    // plb (restore DB)
}

// PITFALLS: 1 (DB manipulation via phb/plb — must ensure DB=0 during DMA setup)
//           4 (stack address depends on E flag — but this routine uses native mode)
//           6 (mode A is 16-bit — all lda/sta are 16-bit unless otherwise noted)
// HELPERS: none (no jsr targets)
// CONTRACT:
//   inputs_reg:  a=16, x=16, y=16
//   inputs_ram:  0x00=2
//   output_ram:  none
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::_13ddd6 ($DD:D6)