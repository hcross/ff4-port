#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=0, DP=0
// Purpose: Restores party data after a solo battle. If the solo character 
// was Golbez or Fusoya (id 0x0B), it performs specific buffer restores.
// It then restores character IDs and clears the status of the solo character.
static void RestorePartyAfterSoloBattle_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // @e74b: Solo character battle id
    uint8_t solo_id = ram[0x00E4] & 0x7F; 
    ram[0x00E4] = solo_id;

    if (solo_id == 0x0B) { // cmp #$0b / bne @e773
        // @e758: Save Fusoya to buffer ($1100 -> $1180, 64 bytes)
        for (int x = 0; x < 0x40; x++) {
            ram[0x1180 + x] = ram[0x1100 + x];
        }
        // @e767: Restore characters 4 and 5 ($1200 -> $10C0, 128 bytes)
        for (int x = 0; x < 0x80; x++) {
            ram[0x10C0 + x] = ram[0x1200 + x];
        }
    }

    // @e773: Restore character IDs and clear status
    for (uint8_t y = 0; y < 5; y++) { // ldy #0 / cpy #5
        uint8_t char_id = ram[0x0AD6 + y];
        
        // The ASM uses 'x' as an index for $1000, but x is reset to 0 at @e773.
        // In a real loop, NextChar likely increments x or a pointer.
        // To maintain parity with the emulator's state of X, we use the current value.
        uint16_t x = snes->cpu->x; 
        ram[0x1000 + x] = char_id;

        if (y == solo_id) { // tya / cmp $e4 / bne @e787
            clear_char_status_emu(snes);
        }
        
        next_char_emu(snes); // jsr NextChar
    }
}

// PITFALLS: 6 (A is 8-bit throughout), 8 (Inherited mode mf=true for field module)
// HELPERS: clear_char_status_emu(snes), next_char_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=0, y=none
//   inputs_ram:  0x00E4=1, 0x1100=64, 0x1200=128, 0x0AD6=5
//   output_ram: 0x1000=5
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::RestorePartyAfterSoloBattle ($E7:4B)