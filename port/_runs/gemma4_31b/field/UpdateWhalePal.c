#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$C3, DP=0
// Logic:
//   Checks if the whale state (ram[0x1704]) is 0x06.
//   If so, it calculates an index based on ram[0x7A] (shifted right twice, masked 0x0E)
//   to select a color pair from the WhalePal table and writes it to 0x0EC7-0x0EC8.
static void UpdateWhalePal_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if (ram[0x1704] != 0x06) { // lda $1704 / cmp #$06 / bne @c3d9
        return;
    }

    uint8_t val = ram[0x7A];
    val = (uint8_t)(val >> 1); // lsr2 (first shift)
    val = (uint8_t)(val >> 1); // lsr2 (second shift)
    val &= 0x0E;               // and #$0e

    // Table access: WhalePal is in ROM. 
    // The asm `lda f:WhalePal,x` uses the index 'val' to offset the table pointer.
    // The index 'x' in 65816 indexed addressing is added to the base address.
    extern const uint8_t WhalePal[]; 
    
    ram[0x0EC7] = WhalePal[val];     // lda f:WhalePal,x / sta $0ec7
    ram[0x0EC8] = WhalePal[val + 1];   // lda f:WhalePal+1,x / sta $0ec8
}

// PITFALLS: 7 (8-bit shift truncation using (uint8_t) casts to ensure parity)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1704=1, 0x7A=1
//   output_ram:  0x0EC7=1, 0x0EC8=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC3
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::UpdateWhalePal ($C3:BD)