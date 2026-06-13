#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$C8 (Field), DP=0
// Logic:
//   Increments a counter at $1703 (max 5).
//   Calls GetTopCharPtr to update X (pointer to char).
//   Reads character from [0x1000 + X].
//   If the character is 0, it loops back to the start (up to 5 times).
//   If a non-zero character is found, it sets $CC to 1 and returns.
static void ValidateTopChar_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // The routine uses a loop based on ram[0x1703].
    // Since it can loop back to @c85e, we use a do-while or similar.
    // Note: $1703 is likely in a different bank or mapping, but based on 
    // the provide context, it's treated as WRAM absolute.
    
    do {
        ram[0x1703]++;                                // inc $1703
        uint8_t counter = ram[0x1703];
        if (counter == 5) {                          // cmp #5 / bne @c86d
            ram[0x1703] = 0;                         // sta $1703
        }

        // jsl GetTopCharPtr (delegated)
        // This routine updates the X register.
        get_top_char_ptr_emu(snes);
        
        // lda $1000,x
        // X is a 16-bit offset. The address is $1000 + X.
        uint16_t x_reg = snes->cpu->x;
        uint8_t char_val = ram[0x1000 + x_reg];       // A 8-bit load
        
        if (char_val != 0) {                         // beq @c85e
            ram[0xCC] = 1;                           // sta $cc
            return;
        }
    } while (true); // beq @c85e loop
}

// PITFALLS: None specifically triggered, but ensured A is 8-bit and X is 16-bit
// to match the memory offset calculation ($1000 + X).
// HELPERS: get_top_char_ptr_emu(snes) — delegates GetTopCharPtr (@c87b)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1703=1
//   output_ram:  0xCC=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xC8
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::ValidateTopChar ($C8:5E)