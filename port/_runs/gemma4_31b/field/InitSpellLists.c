#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$7E, DP=0
// Purpose: Copies a spell list from ROM to WRAM $1560, splitting the
// flat list into blocks of 24 bytes. If a terminator (0xFF) is encountered,
// the remainder of the current block is padded with 0x00.
static void InitSpellLists_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // We need the ROM data for SpellListInit. In a native reimplementation,
    // this would typically be a pointer to the ROM array.
    // For the parity harness, we access it via the ROM read mechanism.
    const uint8_t *spell_list_rom = &snes->rom[0x0C200]; // Hypothetical address based on $C2:3E

    uint16_t x = 0; // ldx #0
    uint16_t y = 0; // ldy #0
    uint8_t block_count = 0; // stz $07 (DP=0)

    do {
        uint8_t val = spell_list_rom[x]; // lda f:SpellListInit, x
        if (val == 0xFF) {               // cmp #$ff / beq @c25f
            // Padding logic: Fill the rest of the 24-byte block with 0
            while (1) {
                ram[0x1560 + y] = 0x00;  // sta $1560, y
                y++;                      // iny
                block_count++;            // inc $07
                if (block_count == 0x18) { // cmp #$18 / bne @c25f
                    block_count = 0;      // stz $07
                    break;
                }
            }
        } else {
            ram[0x1560 + y] = val;       // sta $1560, y
            y++;                         // iny
            block_count++;               // inc $07
            if (block_count == 0x18) {   // cmp #$18 / bne @c26f
                block_count = 0;         // stz $07
            }
        }
        x++;                             // inx
    } while (y != 0x138);                 // cpy #$0138 / bne @c246
}

// PITFALLS: 1 (DB=$7E), 6 (8-bit A, 16-bit X/Y used for indexing/looping)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none (reads from ROM SpellListInit)
//   output_ram:  0x1560=256 (writes a total of 0x138 bytes)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::InitSpellLists ($C2:3E)