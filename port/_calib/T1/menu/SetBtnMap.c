#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$FE, DP=0
// Logic:
//   1. Save current X on stack.
//   2. Use value in A as index into BtnMapTbl to find a mapping value.
//   3. Use that mapping value as an offset to write the original X 
//      (restored from stack) into WRAM at $1A05 + offset.
//
// The use of `longa` before `sta $1a05,x` ensures that the 16-bit X
// is treated as a 16-bit offset from the base $1A05.
static void SetBtnMap_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // phx / sta $43 / ldx $43
    // This sequence effectively saves X and prepares for a table lookup
    // using the current value of A.
    uint16_t original_x = cpu->x; 
    uint8_t index = cpu->a;

    // lda f:BtnMapTbl,x / sta $43 / ldx $43
    // Note: The asm uses the value of A as the index (via $43).
    // Table resides in ROM/Bank. Based on the context of the port, 
    // we access the mapping table via the emulator's memory view.
    uint8_t map_offset = snes->rom[0xBtnMapTbl + index]; 
    
    // longa / pla / sta $1a05,x / shorta
    // Restore X from stack and write it to WRAM.
    // Since we are in a C reimplementation, we just use the captured original_x.
    uint16_t target_addr = 0x1A05 + map_offset;
    write16(ram, target_addr, original_x);
}

// PITFALLS: 6 (Mode A switching: routine explicitly toggles longa/shorta),
//            1 (DB=$FE for this module).
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=16, y=none
//   inputs_ram:  none
//   output_ram:  0x1A05=2 (Offset by BtnMapTbl[a])
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xFE
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: menu::SetBtnMap ($FE:63)