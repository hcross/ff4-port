#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$F3, DP=0
// Logic:
// 1. GetNPCSwitchOffset returns an offset in Y.
// 2. A is set to 1 and shifted left until Y becomes 0.
//    This effectively calculates 2^Y (bitwise: 1 << Y).
// 3. The result is ORed into the bit-field at $0012E0 + X (where X is read from $3D).
static void SetNPCSwitch_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // jsr GetNPCSwitchOffset
    // This routine returns a value in Y (offset) and likely updates X (index)
    get_npc_switch_offset_emu(snes);
    
    uint16_t y = snes->cpu->y;
    uint8_t x = ram[0x3D];
    uint8_t a = 1;

    // Loop to calculate 2^y (asl A / dey / bne)
    while (y != 0) {
        a = (uint8_t)(a << 1); // Pitfall 7: 8-bit truncation
        y--;
    }

    // Calculate target address: f:$0012e0,x
    // Since this is a 'f:' (fixed/absolute) address, we access it relative to the
    // base ROM/RAM mapping provided. In this project, f:$0012e0 maps to WRAM.
    int addr = 0x12E0 + x;
    ram[addr] |= a;
}

// PITFALLS: 7 (asl A in 8-bit mode must truncate to uint8_t)
// HELPERS: get_npc_switch_offset_emu(snes) — delegates GetNPCSwitchOffset @ $F4:14
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3D=1, 0x12E0=1
//   output_ram:  0x12E0=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xF3
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::SetNPCSwitch ($F3:D3)