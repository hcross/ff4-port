#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00, DP=0
// Advances the NPC script pointer (ram[$BC]) to the next non-0xFF byte.
// The loop skips 0xFF markers; once a valid byte is found, the pointer 
// is advanced one more time to point to the next instruction.
static void GetNPCScriptByte_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // ldx $bc
    uint16_t x = read16(ram, 0xBC);
    
    do {
        // inx
        x++;
        
        // lda f:NPCScript,x
        // Using the provided harness read_rom helper for ROM access.
        // f:NPCScript refers to Bank $F, offset NPCScript.
        uint8_t byte = read_rom(snes, 0x0F0000 | NPCScript_Offset); // Conceptual offset
        // Note: In actual parity harness, NPCScript is a constant define.
        // Since the specific offset wasn't provided in the prompt, we use the symbol.
        uint8_t val = read_rom(snes, 0x0F0000 | (NPCScript_Offset + x));
        
        // cmp #$ff / bne @f37a
        if (val != 0xFF) {
            break;
        }
    } while (1);
    
    // inx
    x++;
    
    // stx $bc
    write16(ram, 0xBC, x);
}

// PITFALLS: 6 (Mode A 8-bit comparison), 8 (X is 16-bit)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xBC=2
//   output_ram:  0xBC=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetNPCScriptByte ($F3:78)