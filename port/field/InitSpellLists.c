#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$C2 (assuming field module), DP=0
// Logic:
// Copies spell IDs from the ROM table (f:SpellListInit) to WRAM $1560.
// If a value is $FF, it fills the remaining slots in the current 24-byte ($18) 
// block with $00. It iterates through 312 (0x138) entries.
static void InitSpellLists_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // f:SpellListInit is a ROM address. 
    // In the parity harness, we access ROM via the snes instance.
    const uint8_t *spell_list_init = &snes->rom[0x/*address of f:SpellListInit*/]; 
    // Note: In the actual harness, the specific offset for f:SpellListInit 
    // is resolved based on the disassembly map.

    uint16_t x = 0; // ldx #0
    uint16_t y = 0; // ldy #0
    uint8_t counter = 0; // stz $07

    do {
        uint8_t val = spell_list_init[x]; // lda f:SpellListInit,x
        
        if (val != 0xFF) { // cmp #$ff / beq @c25f
            ram[0x1560 + y] = val; // sta $1560,y
            y++;                   // iny
            counter++;             // inc $07
            if (counter == 0x18) {  // cmp #$18 / bne @c26f
                counter = 0;        // stz $07
            }
        } else { // @c25f
            // Fill remaining slots in the 24-byte block with 0
            do {
                ram[0x1560 + y] = 0; // lda #0 / sta $1560,y
                y++;                  // iny
                counter++;            // inc $07
                if (counter == 0x18) { // cmp #$18 / bne @c25f
                    counter = 0;       // stz $07
                    break;             // exit internal loop
                }
            } while (true);
        }
        x++; // inx
    } while (y != 0x138); // cpy #$0138 / bne @c246
}

// PITFALLS: 6 (Mode A is 8-bit, X/Y are 16-bit), 8 (mf=true inherited)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none (reads from ROM f:SpellListInit)
//   output_ram:  0x1560=312 (bytes)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC2
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::InitSpellLists ($C2:3E)