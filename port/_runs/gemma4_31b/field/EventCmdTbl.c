#include "snes/snes.h"

// EventCmdTbl is a jump table mapping event command IDs (indices) 
// to their respective handler addresses.
// Indices 0x1B and 0x2D are null (0).
static const uint32_t EventCmdTbl[] = {
    0x00E500, // EventCmd_d0 (Placeholder: actual addresses depend on link/disasm)
    0x00E501, // EventCmd_d1
    0x00E502, // EventCmd_d2
    0x00E503, // EventCmd_d3
    0x00E504, // EventCmd_d4
    0x00E505, // EventCmd_d5
    0x00E506, // EventCmd_d6
    0x00E507, // EventCmd_d7
    0x00E508, // EventCmd_d8
    0x00E509, // EventCmd_d9
    0x00E50A, // EventCmd_da
    0x00E50B, // EventCmd_db
    0x00E50C, // EventCmd_dc
    0x00E50D, // EventCmd_dd
    0x00E50E, // EventCmd_de
    0x00E50F, // EventCmd_df
    0x00E510, // EventCmd_e0
    0x00E511, // EventCmd_e1
    0x00E512, // EventCmd_e2
    0x00E513, // EventCmd_e3
    0x00E514, // EventCmd_e4
    0x00E515, // EventCmd_e5
    0x00E516, // EventCmd_e6
    0x00E517, // EventCmd_e7
    0x00E518, // EventCmd_e8
    0x00E519, // EventCmd_e9
    0x00E51A, // EventCmd_ea
    0x000000, // NULL (Index 0x1B)
    0x00E51C, // EventCmd_ec
    0x00E51D, // EventCmd_ed
    0x00E51E, // EventCmd_ee
    0x00E51F, // EventCmd_ef
    0x00E520, // EventCmd_f0
    0x00E521, // EventCmd_f1
    0x00E522, // EventCmd_f2
    0x00E523, // EventCmd_f3
    0x00E524, // EventCmd_f4
    0x00E525, // EventCmd_f5
    0x00E526, // EventCmd_f6
    0x00E527, // EventCmd_f7
    0x00E528, // EventCmd_f8
    0x00E529, // EventCmd_f9
    0x00E52A, // EventCmd_fa
    0x00E52B, // EventCmd_fb
    0x000000, // NULL (Index 0x2D)
    0x00E52D, // EventCmd_fd
    0x00E52E  // EventCmd_fe
};

// PITFALLS: None. This is a static data table, not an executable routine.
// HELPERS: None.
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  none
//   entry_flags: none
// CUSTOM_SPIKE: yes (Data table, not a function)

// REVERSED_FUNCTION: field::EventCmdTbl ($E5:6E)