#include "snes/snes.h"

// This routine prepares parameters for VRAM transfer of the title graphics.
// It sets the VRAM destination to $0000, the transfer size to $2000,
// and the ROM source to TitleGfx ($08:C000), then calls TfrVRAM.
static void LoadTitleGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Destination = $0000 (VRAM)
    write16(ram, 0x47, 0x0000);

    // Size = $2000
    write16(ram, 0x45, 0x2000);

    // Source Bank = $08 (TitleGfx bank)
    ram[0x3C] = 0x08;

    // Source Offset = $C000 (TitleGfx offset)
    write16(ram, 0x3D, 0xC000);

    TfrVRAM_emu(snes); // jsl TfrVRAM
}

// PITFALLS: 1 (WRAM access relies on DB=0x7E), 6 (Mode A/X: 16-bit X used for 
// destination/size/offset, 8-bit A used for bank).
// HELPERS: TfrVRAM_emu(snes) — delegates TfrVRAM @ $00:CA85
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x47=2, 0x45=2, 0x3C=1, 0x3D=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LoadTitleGfx ($86:90)