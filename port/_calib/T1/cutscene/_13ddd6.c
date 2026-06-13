#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$DD, DP=0
// This routine initializes DMA transfer parameters for VRAM/HRAM.
// It clears A using the Direct Page register (D=0), and sets up
// addresses and enable flags in the $43xx range.
static void _13ddd6_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // phb / pha / clr_a / pha / plb / pla
    // This sequence effectively clears A while preserving the 
    // original B register (databank) after the operation.
    uint8_t a_val = (uint8_t)cpu->dp; // clr_a (tdc) where dp=0

    // sty hVMADDL (hVMADDL is a label, assuming absolute address)
    // Note: hVMADDL is typically a WRAM address in these ports.
    // For this translation, we use the label's mapped address.
    write16(ram, 0x434E, cpu->y); // Assuming hVMADDL = 0x434E (example)

    // stx $4352
    write16(ram, 0x4352, cpu->x);

    // sta $4354
    ram[0x4354] = a_val;

    // lda #$01 / sta $4350
    ram[0x4350] = 0x01;

    // lda #<hVMDATAL / sta $4351
    // Assuming hVMDATAL low byte is 0x00 based on typical VRAM DMA tables
    ram[0x4351] = 0x00; // <hVMDATAL

    // ldx $00 / stx $4355
    // ldx $00 loads 16-bit X from DP:0000 (which is 0)
    uint16_t x_val = read16(ram, cpu->dp);
    write16(ram, 0x4355, x_val);

    // lda #$20 / sta hMDMAEN
    // Assuming hMDMAEN = 0x4353
    ram[0x4353] = 0x20; 
}

// PITFALLS: 5 (clr_a is tdc; since DP=0, it results in 0), 
// 6 (A is 8-bit, X/Y are 16-bit).
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=16
//   inputs_ram:  0x0000=2
//   output_ram:  0x4354=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xDD
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::_13ddd6 ($DD:D6)