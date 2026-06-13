#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$9C (implied by code $17xx), DP=0
// Logic:
//   Pushes current map state (ID and coordinate) onto a stack.
//   The stack pointer is tracked at ram[0x172C].
//   If map type ram[0x1700] == 3, it calculates a coordinate from $1702, $1705, $1706.
//   Otherwise, it uses a default coordinate derived from the map type and $1706.
static void PushMapStack_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // phx / plx are balanced, X is preserved
    uint16_t stack_ptr = ram[0x172C]; // ldx $172c
    
    uint8_t map_type = ram[0x1700]; // lda $1700
    if (map_type == 0x03) {         // cmp #$03 / bne @9d0f
        // Case: Map Type 3
        ram[0x172E + stack_ptr] = ram[0x1702]; // lda $1702 / sta $172e,x
        
        // Calculate coordinate: (ram[0x1705] << 1) + ram[0x1706]
        uint8_t val = ram[0x1705];
        val = (uint8_t)(val << 1); // asl6 (Pitfall 7: truncate to 8-bit)
        val = (uint8_t)(val + ram[0x1706]); // clc / adc $1706
        ram[0x172F + stack_ptr] = val; // sta $172f,x
    } else {
        // Case: Map Type != 3
        // Coordinate: ram[0x1700] + 0xFB (equivalent to ram[0x1700] - 5)
        ram[0x172E + stack_ptr] = (uint8_t)(map_type + 0xFB); // clc / adc #$fb / sta $172e,x
        ram[0x172F + stack_ptr] = ram[0x1706]; // lda $1706 / sta $172f,x
    }

    // Store map ID
    ram[0x1730 + stack_ptr] = ram[0x1707]; // lda $1707 / sta $1730,x

    // Increment stack pointer (inx3 is usually a macro for 3 increments or specific word step)
    // Looking at the offsets $172E, $172F, $1730, this is a 3-byte entry.
    stack_ptr += 3; // inx3
    
    if (stack_ptr >= 0x00C0) { // cpx #$00c0 / bcc @9d2f
        stack_ptr = 0;          // ldx #0
    }
    
    ram[0x172C] = (uint8_t)stack_ptr; // stx $172c
}

// PITFALLS: 7 (Arithmetic truncation: asl and adc results cast to uint8_t)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1700=1, 0x1702=1, 0x1705=1, 0x1706=1, 0x1707=1, 0x172C=1
//   output_ram:  0x172C=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x9C
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::PushMapStack ($9C:EB)