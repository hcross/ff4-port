#include "snes/snes.h"

// menu::UpdateCtrl — handles controller input during menus, with special
// L+R combo that masks the A button, and a flag ($F43A) that waits for any
// keypress before clearing itself.  Delegates the battle-extension input
// update (which runs with DP=$0037) to the emulator.
static void UpdateCtrl_c(Snes *snes, uint8_t joy1l, uint8_t joy2l) {
    uint8_t *ram = snes->ram;
    Cpu *c = snes->cpu;

    // $F44A gate — if zero, skip everything
    if (ram[0xF44A] == 0) return;

    // Save DP, set DP=$0037, call UpdateCtrlBattle_ext (delegated)
    uint16_t old_dp = c->dp;
    c->dp = 0x0037;
    c->db = 0x7E;                       // Pitfall 1: battle code expects DB=$7E
    c->mf = true;                        // A 8-bit (inherited from menu caller)
    c->xf = false;                       // X/Y 16-bit
    update_ctrl_battle_ext_emu(snes);
    c->dp = old_dp;                      // pld — restore previous DP

    // Combine joypad 1 & 2 low bytes, check L+R (bits 4-5)
    uint8_t buttons = (joy1l | joy2l) & 0x30;
    if (buttons == 0x30) {               // cmp #$30 / bne → inverted
        ram[0x37] &= 0x7F;               // ignore A button (clear bit 7)
    }

    // $F43A flag: if zero, no keypress wait active
    if (ram[0xF43A] == 0) return;

    // Wait for any keypress in $37 or $38
    if ((ram[0x37] | ram[0x38]) == 0) return;

    ram[0xF43A] = 0;                     // stz $f43a — clear wait flag
}

// PITFALLS: 1 (DB=$7E required before UpdateCtrlBattle_ext call)
// HELPERS: update_ctrl_battle_ext_emu(snes) — delegates UpdateCtrlBattle_ext @ $80:8028
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0xF44A=1, 0xF43A=1, 0x37=1, 0x38=1
//   output_ram:  0x37=1, 0xF43A=1   (both may be modified; 0x38 is read-only here)
//   entry_mode:  mf=true, xf=false, dp=0 (caller's DP), db=0x7E
//   entry_flags: auto (first instruction is lda, which sets Z/N)
//   CUSTOM_SPIKE: yes  (hardware registers f:hSTDCNTRL1L / f:hSTDCNTRL2L
//                        are not in snes->ram; spike must feed joypad bytes)
// REVERSED_FUNCTION: menu::UpdateCtrl ($FD:D0)