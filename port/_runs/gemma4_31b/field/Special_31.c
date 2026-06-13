#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00 (system), DP=0
// This routine sets up a VRAM transfer sequence by writing to SNES 
// VRAM DMA registers ($2115-$2119) using values from CPUCoreVRAMTbl.
static void Special_31_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x0AD4] = 0x60;
    ram[0x0AD5] = 0x48;
    
    _00d7f6_emu(snes);
    WaitVblankShort_emu(snes);

    ram[0x2115] = 0x80;
    
    for (uint16_t x = 0; x < 0x10; x++) {
        if ((x & 0x03) == 0) {
            // Pitfall 7: lsr/asl in 8-bit mode sequence
            uint8_t y = (uint8_t)((x >> 1) << 1);
            
            // CPUCoreVRAMTbl is a ROM table; access via the constant array
            extern const uint8_t CPUCoreVRAMTbl[];
            ram[0x2116] = CPUCoreVRAMTbl[y];
            ram[0x2117] = CPUCoreVRAMTbl[y + 1];
        }
        
        ram[0x2118] = 0x01;
        ram[0x2119] = 0x15;
    }

    WaitVblankEvent_emu(snes);
}

// PITFALLS: 7 (Shift truncation: (uint8_t) cast used to mirror lsr/asl logic)
// HELPERS: _00d7f6_emu(snes), WaitVblankShort_emu(snes), WaitVblankEvent_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x2119=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::Special_31 ($D7:AD)