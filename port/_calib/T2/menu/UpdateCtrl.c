#include "snes/snes.h"

// Updates controller state, handles special key combinations
// and clears battle message wait flag on input
static void UpdateCtrl_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    if (ram[0xF44A] == 0) return;  // beq @fe02
    
    // Push DP, set DP = $0037, call UpdateCtrlBattle_ext
    uint16_t old_dp = snes->cpu->dp;
    snes->cpu->dp = 0x0037;
    update_ctrl_battle_ext_emu(snes);  // jsl UpdateCtrlBattle_ext
    snes->cpu->dp = old_dp;
    
    // Check for L+R button hold (special combo)
    uint8_t ctrl1 = ram[0xF4];  // f:hSTDCNTRL1L
    uint8_t ctrl2 = ram[0xF5];  // f:hSTDCNTRL2L
    uint8_t combo = (ctrl1 | ctrl2) & 0x30;  // and #$30
    if (combo != 0x30) goto check_battle_msg;  // bne @fdf4 (not holding both)
    
    // If L+R held, mask out A button from $37
    ram[0x37] &= 0x7F;  // and #$7f / sta $37
    
check_battle_msg:
    if (ram[0xF43A] == 0) return;  // beq @fe02 (not waiting for keypress)
    
    // If waiting for battle message keypress, check for any input
    if ((ram[0x37] | ram[0x38]) == 0) return;  // beq @fe02 (no keypress)
    
    // Clear the battle message wait flag
    ram[0xF43A] = 0;  // stz $f43a
}

// PITFALLS: 1 (DB=$7E assumed), 8 (A 8-bit, X 16-bit inherited)
// HELPERS: update_ctrl_battle_ext_emu(snes) — delegates UpdateCtrlBattle_ext @ $00:8028
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0xF44A=1, 0xF4=1, 0xF5=1, 0x37=1, 0x38=1, 0xF43A=1
//   output_ram:  0xF43A=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: auto
// REVERSED_FUNCTION: menu::UpdateCtrl ($FD:D0)