#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x7E (WRAM), DP=0
// This routine clears a temporary item list and then filters the inventory
// to populate that list with items meeting specific ID criteria.
//
// Inventory is located at $1440 (2 bytes per item, 96 entries total).
// The filtered list is written to $0712.
static void InitItemList_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // @b2d3: Clear item list (0x0060 bytes)
    for (int x = 0; x < 0x60; x++) {
        ram[0x0712 + x] = 0;
    }

    // @b2e5: Filter inventory into item list
    uint16_t y = 0;
    for (int x = 0; x < 0x60; ) {
        uint8_t item_id = ram[0x1440 + x];
        bool keep = false;

        if (item_id < 0xCE) {
            keep = true; // bcc @b303 not taken -> @b2f8
        } else if (item_id < 0xE7) {
            keep = false; // bcc @b2f8 not taken -> continues to next cmp
        } else if (item_id < 0xEB) {
            keep = true; // bcc @b303 not taken -> @b2f8
        } else if (item_id >= 0xFE) {
            keep = false; // bcs @b303 taken
        } else {
            // Value is between 0xEB and 0xFD (inclusive)
            keep = true; // Falls through to @b2f8
        }
        
        // The ASM logic simplified: 
        // Keep if (id < 0xCE) OR (0xE7 <= id < 0xEB) OR (0xEB <= id < 0xFE)
        // Actually, looking at the BCC/BCS flow:
        // id < 0xCE -> @b303 (skip)
        // 0xCE <= id < 0xE7 -> @b2f8 (KEEP)
        // 0xE7 <= id < 0xEB -> @b303 (skip)
        // 0xEB <= id < 0xFE -> @b2f8 (KEEP)
        // id >= 0xFE -> @b303 (skip)

        // CORRECTED logic based on ASM branch targets:
        bool match = false;
        if (item_id < 0xCE) {
            match = false; // bcc @b303
        } else if (item_id < 0xE7) {
            match = true;  // bcc @b2f8
        } else if (item_id < 0xEB) {
            match = false; // bcc @b303
        } else if (item_id >= 0xFE) {
            match = false; // bcs @b303
        } else {
            match = true;  // falls through to @b2f8
        }

        if (match) {
            ram[0x0712 + y] = ram[0x1440 + x];     // sta $0712,y
            ram[0x0713 + y] = ram[0x1441 + x];     // sta $0713,y
            y += 2;                                // iny2
        }
        x += 2; // inx2 (X is 16-bit)
    }
}

// PITFALLS: 6 (Mode A 8-bit), 8 (Inherited mf=true), 3 (CMP/BCC logic flow)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1440=1 (inventory start)
//   output_ram:  0x0712=1 (item list start)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::InitItemList ($B2:D3)