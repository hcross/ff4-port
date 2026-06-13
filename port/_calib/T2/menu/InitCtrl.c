#include "snes/snes.h"

// Entry mode: A 16-bit (mf=0), X/Y 16-bit (xf=0), DB=$00, DP=0
// All inputs/outputs in WRAM (no register I/O — convention menu):
//   in : ram[$16A9], ram[$1A3A], ram[$1A3B], ram[$1A37], ram[$1A38], ram[$1A39], ram[$DD]
//   out: ram[$1A1D..$1A32] updated with button mappings
static void InitCtrl_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // Save previous DP and DB
    uint16_t old_dp = cpu->dp;
    uint8_t old_db = cpu->db;

    // Set DP to menu_dp (assumed to be zero from context)
    cpu->dp = 0;
    cpu->db = 0x7E;  // WRAM bank for data access

    // MVN copies 24 bytes from BtnDefault to $1A05 and $1A1D
    cpu->mf = false;  // longa
    cpu->a = 0x0017;  // byte count - 1
    cpu->x = 0;       // source offset (BtnDefault is at 0 in bank)
    cpu->y = 0x1A05;  // destination offset
    // Simulate MVN #^BtnDefault,#$7e (copy 24 bytes)
    for (int i = 0; i < 24; i++) {
        ram[0x7E0000 + 0x1A05 + i] = ram[0x000000 + 0 + i];
    }
    cpu->x = 0;
    cpu->y = 0x1A1D;
    for (int i = 0; i < 24; i++) {
        ram[0x7E0000 + 0x1A1D + i] = ram[0x000000 + 0 + i];
    }

    cpu->mf = true;  // shorta
    ram[0x1A64] = ram[0x16A9];

    uint8_t tmp = ram[0x1A3A] << 1;
    ram[0x43] = tmp;
    uint16_t x = (uint16_t)tmp;

    cpu->mf = false;  // longa
    write16(ram, 0x1A2D, 0);  // clear A, B, X, Y, and select buttons
    write16(ram, 0x1A1D, 0);
    write16(ram, 0x1A2F, 0);
    write16(ram, 0x1A1F, 0);
    write16(ram, 0x1A21, 0);

    // Set action for L button
    ram[0x1A31] = ram[0x000000 + x];  // f:BtnAction,x

    cpu->mf = true;  // shorta
    tmp = ram[0x1A3B] << 1;
    ram[0x43] = tmp;
    x = (uint16_t)tmp;

    cpu->mf = false;  // longa
    // Set action for start button
    ram[0x1A23] = ram[0x000000 + x];  // f:BtnAction,x

    cpu->mf = true;  // shorta
    uint8_t a = ram[0x1A37];
    uint16_t x_param = 0x0080;  // confirm
    set_btn_map_emu(snes, a, x_param);

    a = ram[0x1A38];
    x_param = 0x8000;  // cancel
    set_btn_map_emu(snes, a, x_param);

    a = ram[0x1A39];
    x_param = 0x0040;  // menu
    set_btn_map_emu(snes, a, x_param);

    ram[0x04] = 0xFF;
    ram[0x05] = 0xFF;

    uint8_t dd = ram[0xDD];
    ram[0x08] = dd;
    ram[0x09] = dd;

    // Restore DP and DB
    cpu->dp = old_dp;
    cpu->db = old_db;

    // tdc / xba (clr_a / xba) sets A=0
    cpu->a = 0;
}

// PITFALLS: 1 (DB must be $7E for WRAM access), 6 (A/X mode switching),
//           7 (arithmetic truncation in 8-bit mode), 8 (mode A/X heritage)
// HELPERS: set_btn_map_emu(snes, a, x) — delegates SetBtnMap @ $FD:FE63
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x16a9=1, 0x1a3a=1, 0x1a3b=1, 0x1a37=1, 0x1a38=1, 0x1a39=1, 0xdd=1
//   output_ram:  0x1a1d=2
//   entry_mode:  mf=false, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: menu::InitCtrl ($FD:D9)