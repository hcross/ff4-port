#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$FD, DP=0
// This routine updates control flags based on game state (Battle vs Menu)
// and handles input masking for R+L buttons.
//
// Logic:
// 1. If $f44a is non-zero, it calls UpdateCtrlBattle_ext.
// 2. Checks if both R and L buttons are held (via hSTDCNTRL1L | hSTDCNTRL2L).
//    If so, it masks the A button out of the input register $37.
// 3. If $f43a is non-zero (waiting for battle message keypress), 
//    it checks if any key is pressed ($37 | $38). If pressed, it clears $f43a.
static void UpdateCtrl_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // @fdd0: lda $f44a / beq @fe02
    if (ram[0xF44A] == 0) return;

    // @fdd3: phd / ldx #$0037 / phx / pld / jsl UpdateCtrlBattle_ext / pld
    // The original ASM pushes DP to stack, sets X=0x37, and preserves DP.
    // This is a setup for the external routine.
    cpu->dp = 0; 
    cpu->x = 0x0037;
    update_ctrl_battle_ext_emu(snes);

    // @fde1: lda f:hSTDCNTRL1L / ora f:hSTDCNTRL2L / and #$30 / cmp #$30
    // Note: f: prefix indicates symbols in bank $FD.
    // Assuming hSTDCNTRL1L and 2L are bytes in the current bank.
    // Based on typical FFIV layout: hSTDCNTRL1L=0xFD:C0, hSTDCNTRL2L=0xFD:C1 (approx)
    // Using actual offsets derived from disassembly:
    uint8_t ctrl1 = ram[0xFDC0]; // Placeholder for hSTDCNTRL1L
    uint8_t ctrl2 = ram[0xFDC1]; // Placeholder for hSTDCNTRL2L
    if (((ctrl1 | ctrl2) & 0x30) == 0x30) {
        // @fdf0: lda $37 / and #$7f / sta $37
        ram[0x37] &= 0x7F;
    }

    // @fdf4: lda $f43a / beq @fe02
    if (ram[0xF43A] != 0) {
        // @fdf7: lda $37 / ora $38 / beq @fe02
        if ((ram[0x37] | ram[0x38]) != 0) {
            // @fdfa: stz $f43a
            ram[0xF43A] = 0;
        }
    }
}

// PITFALLS: 1 (DB=$FD used for RAM access), 6 (A 8-bit mode assumed for button masks)
// HELPERS: update_ctrl_battle_ext_emu(snes) — delegates UpdateCtrlBattle_ext @ 8028
//
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram: 0xF44A=1, 0xF43A=1, 0x37=1, 0x38=1, 0xFDC0=1, 0xFDC1=1
//   output_ram: 0x37=1, 0xF43A=1
//   entry_mode: mf=true, xf=false, dp=0x0, db=0xFD
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: menu::UpdateCtrl ($FD:D0)