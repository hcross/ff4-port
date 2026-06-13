#include "snes/snes.h"

// InitCtrl initializes the menu control mapping, copying default button 
// configurations and setting up specific action mappings for L, Start, 
// and other menu buttons.
static void InitCtrl_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // MVN blocks: Copy BtnDefault (from ROM/Bank) to WRAM
    // First MVN: 0x17 bytes from BtnDefault to $7E:1A05
    // Second MVN: 0x17 bytes from BtnDefault to $7E:1A1D
    // Note: These are ROM-to-RAM copies. Since we are in C, we simulate the effect.
    // Assuming BtnDefault is a known constant array in the ROM image.
    for (int i = 0; i < 0x17; i++) {
        ram[0x1A05 + i] = snes->rom[BTN_DEFAULT_OFFSET + i];
        ram[0x1A1D + i] = snes->rom[BTN_DEFAULT_OFFSET + i];
    }

    // shorta: Mode A = 8-bit
    uint8_t val_16a9 = ram[0x16A9];
    ram[0x1A64] = val_16a9;

    uint8_t val_1a3a = ram[0x1A3A];
    uint8_t shifted_1a3a = (uint8_t)(val_1a3a << 1); // asl
    ram[0x43] = shifted_1a3a;

    // longa: Mode A = 16-bit
    uint8_t x_idx = ram[0x43];
    
    // stz block: Clear specific menu control registers
    // stz $1a2d, $1a1d, $1a2f, $1a1f, $1a21 (16-bit zeros)
    write16(ram, 0x1A2D, 0);
    write16(ram, 0x1A1D, 0);
    write16(ram, 0x1A2F, 0);
    write16(ram, 0x1A1F, 0);
    write16(ram, 0x1A21, 0);

    // lda f:BtnAction,x -> sta $1a31
    // BtnAction is in ROM. Index x is based on ram[0x43].
    ram[0x1A31] = snes->rom[BTN_ACTION_OFFSET + x_idx];

    // shorta: Mode A = 8-bit
    uint8_t val_1a3b = ram[0x1A3B];
    uint8_t shifted_1a3b = (uint8_t)(val_1a3b << 1); // asl
    ram[0x43] = shifted_1a3b;

    // longa: Mode A = 16-bit
    x_idx = ram[0x43];
    ram[0x1A23] = snes->rom[BTN_ACTION_OFFSET + x_idx];

    // shorta: Mode A = 8-bit
    // SetBtnMap(ram[0x1A37], 0x0080) - Confirm
    uint8_t btn_confirm = ram[0x1A37];
    snes->cpu->a = btn_confirm;
    snes->cpu->x = 0x0080;
    set_btn_map_emu(snes);

    // SetBtnMap(ram[0x1A38], 0x8000) - Cancel
    uint8_t btn_cancel = ram[0x1A38];
    snes->cpu->a = btn_cancel;
    snes->cpu->x = 0x8000;
    set_btn_map_emu(snes);

    // SetBtnMap(ram[0x1A39], 0x0040) - Menu
    uint8_t btn_menu = ram[0x1A39];
    snes->cpu->a = btn_menu;
    snes->cpu->x = 0x0040;
    set_btn_map_emu(snes);

    // Standard menu/input register initialization
    ram[0x04] = 0xFF;
    ram[0x05] = 0xFF;
    uint8_t repeat_rate = ram[0xDD];
    ram[0x08] = repeat_rate;
    ram[0x09] = repeat_rate;
}

// PITFALLS: 6 (Strict adherence to longa/shorta for register size), 
//           7 (asl truncation to 8-bit in shorta mode)
// HELPERS: set_btn_map_emu(snes) — delegates SetBtnMap @ $FE:63
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  0x16A9=1, 0x1A3A=1, 0x1A3B=1, 0x1A37=1, 0x1A38=1, 0x1A39=1, 0xDD=1
//   output_ram: 0x1A64=1, 0x43=1, 0x1A31=1, 0x1A23=1, 0x04=1, 0x05=1, 0x08=1, 0x09=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: menu::InitCtrl ($FD:D9)