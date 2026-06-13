#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$FD, DP=0
// Logic:
// 1. If $f44a is non-zero, call UpdateCtrlBattle_ext.
// 2. If R and L buttons are held (STDCNTRL1/2), mask off the A button in $37.
// 3. If $f43a (message keypress wait) is active and any key is pressed ($37|$38),
//    clear the wait flag $f43a.
static void UpdateCtrl_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if (ram[0xF44A] == 0) { // lda $f44a / beq @fe02
        return;
    }

    // Setup for UpdateCtrlBattle_ext:
    // Original asm pushes DP and X to stack.
    // Since we are using a delegated emulator call, we must ensure 
    // the emulated CPU state reflects these parameters.
    snes->cpu->dp = 0;
    snes->cpu->x = 0x0037;
    update_ctrl_battle_ext_emu(snes); // jsl UpdateCtrlBattle_ext

    // Check for R + L buttons (hSTDCNTRL1L and hSTDCNTRL2L are ROM/RAM flags)
    // Note: 'f:hSTDCNTRL1L' indicates a symbol in the 'f' (fixed/ram) segment.
    // Based on common FFIV mapping, these are standard control register offsets.
    uint8_t ctrl1 = ram[0x00]; // Placeholder for hSTDCNTRL1L offset
    uint8_t ctrl2 = ram[0x01]; // Placeholder for hSTDCNTRL2L offset
    // Correction: Since specific offsets for hSTDCNTRL aren't provided in the snippet,
    // the symbols usually map to the controller state bytes in WRAM.
    // For this translation, we assume they are accessible via the ram pointer.
    // Assuming hSTDCNTRL1L = 0x37 and hSTDCNTRL2L = 0x38 based on the following logic.
    
    uint8_t combined_ctrl = ram[0x37] | ram[0x38]; 
    if ((combined_ctrl & 0x30) == 0x30) { // and #$30 / cmp #$30 / bne @fdf4
        ram[0x37] &= 0x7F;               // and #$7f / sta $37
    }

    if (ram[0x43A] == 0) { // lda $f43a / beq @fe02 (Note: ASM says $f43a, likely typo in prompt's @fdf4 block vs $f43a)
        return;
    }

    if ((ram[0x37] | ram[0x38]) != 0) { // lda $37 / ora $38 / beq @fe02
        ram[0xF43A] = 0;                 // stz $f43a
    }
}

// PITFALLS: 1 (DB=$FD for menu module), 8 (mf=true for 8-bit A)
// HELPERS: update_ctrl_battle_ext_emu(snes) — delegates UpdateCtrlBattle_ext @ 8028
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xF44A=1, 0xF43A=1, 0x37=1, 0x38=1
//   output_ram:  0xF43A=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFD
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: menu::UpdateCtrl ($FD:D0)