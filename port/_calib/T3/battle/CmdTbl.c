#include "snes/snes.h"

// Battle command dispatch table at $B3:006C (ROM data, not executable code).
// 37 entries indexed by command ID (0x00–0x24), each a 16-bit offset in bank $B3.
// 0 = unused/invalid command slot.
// Total size: 74 bytes.
//
// C code that dispatches commands reads from ROM at runtime:
//   uint16_t handler = read16(snes->rom, 0xB36C + cmd_id * 2);
//   if (handler) run_emulated_func(snes, 0xB30000 | handler);

static const uint16_t kCmdTbl[37] = {
    /* 0x00 */ CMD_00_ADDR, CMD_01_ADDR, CMD_02_ADDR, 0, 0,
    /* 0x05 */ CMD_05_ADDR, CMD_06_ADDR, CMD_07_ADDR, CMD_08_ADDR, CMD_09_ADDR,
    /* 0x0a */ CMD_0A_ADDR, CMD_0B_ADDR, CMD_0C_ADDR, CMD_0D_ADDR, CMD_0E_ADDR,
    /* 0x0f */ CMD_0F_ADDR, CMD_10_ADDR, CMD_11_ADDR, CMD_12_ADDR, CMD_13_ADDR,
    /* 0x14 */ CMD_14_ADDR, 0, CMD_16_ADDR, CMD_17_ADDR, 0,
    /* 0x19 */ CMD_19_ADDR, CMD_1A_ADDR, CMD_1B_ADDR, CMD_1C_ADDR, CMD_1D_ADDR,
    /* 0x1e */ CMD_1E_ADDR, CMD_1F_ADDR, CMD_20_ADDR, CMD_21_ADDR, CMD_22_ADDR,
    /* 0x23 */ 0, CMD_24_ADDR
};

// PITFALLS: (none — pure data table, no executable code)
// HELPERS: (none)
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  n/a (data table)
//   entry_flags: n/a
// CUSTOM_SPIKE: yes

// REVERSED_FUNCTION: battle::CmdTbl ($B3:6C)