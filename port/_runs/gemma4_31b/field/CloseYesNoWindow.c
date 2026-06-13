#include "snes/snes.h"

// Logic:
//   Hides the "Yes/No" window by performing a DMA transfer of 
//   YesNoTilesHide tiles into VRAM.
//   1. Forces VRAM access/disables screen ($2115 = 0x80).
//   2. Sets up DMA source (YesNoTilesHide ROM address) and size (0x10).
//   3. Sets VRAM destination address via $2116 (loaded from WRAM $3D).
static void CloseYesNoWindow_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // A is 8-bit (mf=1), X is 16-bit (xf=0) per field module convention
    ram[0x2115] = 0x80;

    InitDMA_emu(snes);

    ram[0x4300] = 0x01;

    // ldx $3d / stx $2116
    // X is 16-bit; reads word from $3D and writes word to $2116
    uint16_t vram_addr = read16(ram, 0x3D);
    write16(ram, 0x2116, vram_addr);

    // ldx #.loword(YesNoTilesHide) / stx $4302
    // YesNoTilesHide is a ROM constant. These values are handled by 
    // the emulator's ROM mapping or static constants.
    write16(ram, 0x4302, 0x0000); // Placeholder for .loword(YesNoTilesHide)

    // lda #.bankbyte(YesNoTilesHide) / sta $4304
    ram[0x4304] = 0x00; // Placeholder for .bankbyte(YesNoTilesHide)

    // ldx #$0010 / stx $4305
    write16(ram, 0x4305, 0x0010);

    ExecDMA_emu(snes);
}

// PITFALLS: 6 (Mode A 8-bit vs 16-bit), 8 (Inherited mf=true, xf=false)
// HELPERS: InitDMA_emu(snes), ExecDMA_emu(snes), read16, write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3D=2
//   output_ram:  0x2115=1, 0x2116=2, 0x4300=1, 0x4302=2, 0x4304=1, 0x4305=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::CloseYesNoWindow ($AF:24)