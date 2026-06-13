#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0, DP=0
// This routine initializes various hardware registers and system variables 
// related to map rendering, Mode 7, and animation state.
static void InitMapRAM_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // lda #$80 / sta hINIDISP (Assuming hINIDISP is a hardware register/RAM offset)
    // In the context of the field module, these offsets are often in the 0x00-0xFF range (DP=0)
    ram[0x00] = 0x80; // hINIDISP (Placeholder: specific offset depends on include/macros.inc)
    
    // stz hHDMAEN / stz hNMITIMEN
    ram[0x01] = 0;    // hHDMAEN (Placeholder)
    ram[0x02] = 0;    // hNMITIMEN (Placeholder)
    
    cpu->i = true;    // sei (disable interrupts)

    reset_sprites_emu(snes); // jsr ResetSprites

    // Clear animation and map state variables (DP=0, 8-bit zeros)
    ram[0x7A] = 0;    // animation frame counter
    ram[0x94] = 0;
    ram[0xEB] = 0;
    ram[0xE9] = 0;
    ram[0xEB] = 0;    // Duplicate in asm
    ram[0xEC] = 0;
    ram[0xED] = 0;
    ram[0xEA] = 0;    // show map name
    ram[0xE7] = 0;
    ram[0xE8] = 0;
    ram[0xD4] = 0;
    ram[0xAB] = 0;
    ram[0xCF] = 0;
    ram[0xDA] = 0;
    ram[0xC9] = 0;
    ram[0xC4] = 0;
    ram[0xC1] = 0;

    // lda #1 / sta sequence
    ram[0x54] = 1;
    ram[0x55] = 1;
    ram[0x50] = 1;
    ram[0x51] = 1;
    ram[0x52] = 1;
    ram[0x53] = 1;
    ram[0x56] = 1;
    ram[0x57] = 1;

    // Clear Mode 7 related variables
    ram[0x66] = 0;
    ram[0x67] = 0;
    ram[0x68] = 0;
    ram[0x69] = 0;

    // lda #$10 / sta $ad (mode 7 zoom level)
    ram[0xAD] = 0x10;

    // ldx #0 / stx $06fb (mode 7 rotation angle)
    // X is 16-bit (xf=0), so this writes two bytes (little endian)
    write16(ram, 0x06FB, 0);
}

// PITFALLS: None (Direct store/load sequence, no complex branching or mode shifts)
// HELPERS: reset_sprites_emu(snes) - delegates ResetSprites @ 8980
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x06FB=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::InitMapRAM ($83:4E)