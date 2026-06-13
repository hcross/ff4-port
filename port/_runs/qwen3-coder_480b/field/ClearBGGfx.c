#include "snes/snes.h"

// ClearBGGfx: Clears background graphics by setting up DMA to fill VRAM
// with a fixed value ($06). Uses HDMA channel 0 to write $06 to VRAM
// range $0000-$17FF (3000 bytes).
static void ClearBGGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    ram[0x420B] = 0;              // stz hMDMAEN
    ram[0x2115] = 0x80;           // sta hVMAINC
    ram[0x4300] = 0x08;           // sta $4300
    ram[0x4301] = 0x19;           // sta $4301
    write16(ram, 0x2116, 0x0000); // stx hVMADDL
    ram[0x06] = 0;                // stz $06 (fixed value for clearing)
    write16(ram, 0x4302, 0x0606); // stx $4302
    ram[0x4304] = 0;              // stz $4304
    write16(ram, 0x4305, 0x1800); // stx $4305 (3000 bytes)
    ram[0x420B] = 1;              // sta hMDMAEN
}

// PITFALLS: 1 (DB must be $7E for WRAM access — but this routine only
// accesses hardware registers, so DB is irrelevant), 8 (no explicit
// mode directives — A/X mode inherited from caller, but this routine
// only uses 8-bit stores, so mode is irrelevant)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=auto, xf=auto, dp=auto, db=auto
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::ClearBGGfx ($B1:0014)