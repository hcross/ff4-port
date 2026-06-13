#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0, DP=0
// This routine configures the SNES DMA to clear the first $3000 bytes of VRAM.
// It uses the DMA registers at $4300-$4305 and the VMA (VRAM Memory Address) 
// and MDMAEN (DMA Enable) hardware registers.
static void ClearBGGfx_c(Snes *snes) {
    // Memory-mapped I/O registers are accessed via snes->ram or direct mapping.
    // In the target emulator context, these are generally handled by 
    // writing to the snes->ram array at their hardware addresses.
    uint8_t *ram = snes->ram;

    // stz hMDMAEN (hMDMAEN is typically $42C0)
    ram[0x42C0] = 0;

    // lda #$80 / sta hVMAINC (hVMAINC is typically $4207)
    ram[0x4207] = 0x80;

    // lda #$08 / sta $4300 (DMA Source Address)
    ram[0x4300] = 0x08;

    // lda #$19 / sta $4301 (DMA Source Address high)
    ram[0x4301] = 0x19;

    // ldx #$0000 / stx hVMADDL (hVMADDL is typically $4202)
    write16(ram, 0x4202, 0x0000);

    // stz $06 (Write 0 to WRAM $06 to be used as the fill value)
    ram[0x06] = 0;

    // ldx #$0606 / stx $4302 (DMA Destination Address)
    write16(ram, 0x4302, 0x0606);

    // stz $4304 (DMA Destination Address High/Extra)
    write16(ram, 0x4304, 0x0000);

    // ldx #$1800 / stx $4305 (DMA Word Count: $1800 words = $3000 bytes)
    write16(ram, 0x4305, 0x1800);

    // lda #1 / sta hMDMAEN (Enable DMA)
    ram[0x42C0] = 1;
}

// PITFALLS: None. Straightforward sequence of I/O register writes.
// HELPERS: read16/write16
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: auto
//   CUSTOM_SPIKE: yes (Hardware register writes; cannot be verified via WRAM parity)

// REVERSED_FUNCTION: field::ClearBGGfx ($B1:14)