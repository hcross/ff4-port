#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=0, DP=0 (Field module default)
// Logic: 
// 1. Calls GetNPCSwitchOffset to get an offset in Y.
// 2. The routine contains a loop that effectively clears the lower bits of a byte
//    starting from the calculated offset. It continues until Y reaches 0.
// 3. Finally, it calculates a target address in bank $00 (f:$0012e0 + X) 
//    and performs a bitwise AND with $FE to clear the LSB.
static void ClearNPCSwitch_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // jsr GetNPCSwitchOffset
    // This routine typically sets Y. Since we are translating, we delegate.
    get_npc_switch_offset_emu(snes);
    uint16_t y = snes->cpu->y;

    // lda #$fe
    uint8_t a = 0xFE;

    // Loop @f3a5
    while (y != 0) { // cpy #0 / beq @f3b0
        // sec / rol / dey
        // This sequence is a manual bit-shift/rotation of the 8-bit A register.
        // However, A is initialized to $FE (1111 1110).
        // sec (C=1) -> rol A -> A becomes 1111 1101 (0xFD), C becomes 1.
        // This repeats until Y=0.
        a = (uint8_t)((a << 1) | 1); // Simplified: SEC + ROL A (8-bit)
        y--;
    }

    // ldx $3d
    uint16_t x = ram[0x3D];
    
    // and f:$0012e0,x
    // target = bank 0, offset 0x12E0 + x
    // Note: LakeSnes ram is a flat array; we must handle bank 0 mapping.
    // f:$0012e0 is likely an absolute address in the SNES memory map.
    uint8_t target_val = ram[0x12E0 + x];
    target_val &= a; // a was modified in the loop, but the ASM says 'and f:$0012e0,x'
    // Wait, the ASM is 'and f:$0012e0,x' followed by 'sta f:$0012e0,x'.
    // The accumulator 'a' is the operand for AND.
    
    ram[0x12E0 + x] = target_val;
}

// PITFALLS: 7 (8-bit rotation truncated to uint8_t), 8 (Field module defaults: mf=true, xf=false)
// HELPERS: get_npc_switch_offset_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x3D=1, 0x12E0=1
//   output_ram:  0x12E0=1
//   entry_mode:  mf=true, xf=false, dp=0, db=0
//   entry_flags: z=auto, n=auto

// REVERSED_FUNCTION: field::ClearNPCSwitch ($F3:A0)