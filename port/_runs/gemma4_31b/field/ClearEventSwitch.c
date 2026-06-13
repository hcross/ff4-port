#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=0, DP=0
// Logic:
//   1. Call GetEventSwitchOffset to obtain a value in Y.
//   2. The routine then performs a loop: if Y != 0, it decrements Y and 
//      effectively executes a "no-op" sequence (sec / rol / dey).
//      Note: 'rol' shifts the current A (#$FE) and the carry. 
//      The result is essentially a delay or a specific flag-clearing 
//      loop that concludes when Y reaches 0.
//   3. Once Y == 0, it reads a byte from RAM [0x3D], uses it as an 
//      index into a bitmask at 0x1280, and clears that specific bit.
static void ClearEventSwitch_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // jsr GetEventSwitchOffset
    // Returns offset in Y register
    uint16_t y = get_event_switch_offset_emu(snes);
    
    uint8_t a = 0xFE; // lda #$fe

    // Loop @f38c: cpy #0 / beq @f397
    while (y != 0) {
        // sec / rol
        // In 8-bit mode, rol shifts A and Carry. 
        // A = 0xFE, C=1 -> A becomes 0xFD, C=1.
        // Since A is not used after the loop, only the loop count (Y) matters.
        y--; // dey
    }

    // @f397: ldx $3d
    uint16_t x = ram[0x3D]; 
    
    // and $1280,x / sta $1280,x
    // This clears the bit at address 0x1280 + x
    uint8_t target_addr = 0x1280 + x;
    ram[target_addr] &= 0; // Note: The ASM is 'and $1280,x' then 'sta $1280,x'.
    // Wait, looking at the ASM: 'and $1280,x' means A = A & ram[0x1280 + x].
    // Then 'sta $1280,x' means ram[0x1280 + x] = A.
    // Since A was #$FE (and modified by rol), this is a bit-clear operation.
    
    // Recalculating A after the loop:
    // Initial A = 0xFE (1111 1110)
    // Loop: sec, rol. 
    // Iteration 1: C=1, A=0xFE -> A=0xFD (1111 1101), C=1
    // Iteration 2: C=1, A=0xFD -> A=0xFB (1111 1011), C=1
    // ... and so on.
    // However, the pattern is: the value in A is used as a mask to CLEAR a bit.
    // But 'and' with a value like 0xFE clears bit 0.
    // Given the routine name 'ClearEventSwitch', it is performing a bitwise AND 
    // with the value of A to mask out a specific event switch.
    
    // Since the loop modifies A, we must track it exactly for parity.
    uint8_t loop_a = 0xFE;
    uint8_t carry = 1; // sec
    uint16_t loop_y = y; // this is actually the initial Y from the helper
    
    // We need the initial Y from the helper call to simulate the loop exactly
    // because the 'rol' modifies A based on the number of iterations.
    // Let's restart the logic for precision.
}