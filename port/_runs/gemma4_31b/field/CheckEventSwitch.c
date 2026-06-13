#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$F3 (assumed for field), DP=0
// Logic:
//   Checks a bitmask in the event switch RAM area. 
//   Calculates an offset via GetEventSwitchOffset, then iterates 
//   through the bits of the byte at that offset until a zero bit is found 
//   or the byte is exhausted.
//   Returns 1 (A=1) if the switch is active, 0 otherwise.
static uint8_t CheckEventSwitch_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // phx: preserve X
    // jsr GetEventSwitchOffset: delegated
    uint16_t offset = get_event_switch_offset_emu(snes);

    // ldx $3d: Load offset (Wait, ASM says ldx $3d, likely referring to DP $00:3D)
    // However, GetEventSwitchOffset usually puts the result in A or a specific RAM loc.
    // The ASM is: ldx $3d -> lda $1280,x
    // In the context of field scripts, $3D is a common scratchpad RAM address.
    uint8_t x_val = ram[0x3D]; 
    
    // lda $1280,x: Offset into the event switch table
    uint8_t val = ram[0x1280 + x_val];
    
    // The ASM loop @f3f6 uses cpy #0. 
    // But A was just loaded. The loop uses 'cpy #0' (comparing Y to 0).
    // Let's re-examine the ASM sequence:
    // ldx $3d       -> X = ram[0x3D]
    // lda $1280,x   -> A = ram[0x1280 + X]
    // @f3f6: cpy #0 -> Compare Y to 0
    //       beq @f400 -> if Y == 0, exit loop
    //       lsr     -> Shift A right (the bit we are checking)
    //       dey     -> decrement Y
    //       jmp @f3f6
    
    // Critical observation: Y is not initialized in this routine. 
    // It is inherited from the caller. This determines the bit-depth check.
    uint8_t a = val;
    uint16_t y = snes->cpu->y;

    while (y != 0) { // cpy #0 / beq @f400
        if ((a & 1) != 0) {
            // The ASM does not actually check the carry for the branch, 
            // it only shifts and decrements Y.
            // Wait, the logic at @f400 is 'lsr / lda #0 / adc #0'.
            // If we break the loop via 'cpy #0', we hit the final lsr.
            // If the loop continues, it's just consuming Y.
        }
        a = (uint8_t)(a >> 1); // Pitfall 7: truncate to 8-bit
        y--;                   // dey
    }

    // @f400:
    a = (uint8_t)(a >> 1);     // lsr
    
    // lda #0 / adc #0: This is a common pattern to set A to the Carry flag.
    // Since 'lsr' puts the 0-th bit into Carry, and we then do 0 + Carry,
    // the result is simply the bit that was shifted out.
    uint8_t result = (a & 1) ? 0 : 0; // This is a placeholder; the logic is (Carry)
    
    // Correcting the logic: the loop @f3f6 shifts A until Y is 0.
    // Then one final LSR is performed. The result is the C flag.
    // Therefore, this function returns the bit at position (Y+1).
    
    // To match the ASM exactly:
    // 1. Load A = ram[0x1280 + ram[0x3D]]
    // 2. Repeat Y times: { A >>= 1 }
    // 3. A >>= 1
    // 4. Return Carry (the bit shifted out in step 3)
    
    uint8_t current_a = ram[0x1280 + ram[0x3D]];
    uint16_t current_y = snes->cpu->y;
    
    while (current_y > 0) {
        current_a >>= 1;
        current_y--;
    }
    
    uint8_t final_bit = current_a & 1;
    current_a >>= 1; // The final LSR at @f400
    
    return final_bit; 
}

// PITFALLS: 7 (lsr A truncated to 8-bit), 8 (Inherited Y register size/value)
// HELPERS: get_event_switch_offset_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=16
//   inputs_ram:  0x3D=1, 0x1280=1 (indirect via 0x3D)
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xF3
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes
// REVERSED_FUNCTION: field::CheckEventSwitch ($F3:ED)