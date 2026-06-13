#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x7E, DP=0
// Entry: A = button index (8-bit), X = base offset (16-bit)
// Logic:
//   1. Push X to stack
//   2. Use A as index into BtnMapTbl (ROM table)
//   3. Load byte from BtnMapTbl[X] into $43
//   4. Reload X from $43 (new offset)
//   5. Pop original X from stack into A (16-bit)
//   6. Store A at $1A05 + X (new offset)
static void SetBtnMap_c(Snes *snes, uint8_t btn_index) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    uint16_t old_x = cpu->x;              // phx
    ram[0x43] = btn_index;                // sta $43
    uint16_t tbl_index = ram[0x43];       // ldx $43 (X 16-bit)
    uint8_t map_value = BtnMapTbl[tbl_index]; // lda f:BtnMapTbl,x
    ram[0x43] = map_value;                // sta $43
    uint16_t new_x = ram[0x43];           // ldx $43 (X 16-bit)
    cpu->mf = false;                      // longa
    cpu->a = old_x;                       // pla (16-bit A)
    write16(ram, 0x1A05 + new_x, cpu->a); // sta $1a05,x
    cpu->mf = true;                       // shorta
}

// PITFALLS: 6 (mode A 8-bit/16-bit transitions via longa/shorta),
//           1 (DB=0x7E assumed for RAM access)
// HELPERS: none (pure C implementation)
// CONTRACT:
//   inputs_reg:  a=8, x=16, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: none
// REVERSED_FUNCTION: menu::SetBtnMap ($FE:63)