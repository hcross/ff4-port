#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=$00 (implied by absolute addresses), DP=0
// Purpose: Iterates through the treasures of the current map, calculates their offsets,
// checks their switches, and potentially updates trigger values in memory.
static void InitTreasures_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if (ram[0x0711] == 0) {             // lda $0711 / bne @9b84 (Note: asm says bne but logic is "return if no treasures")
        // Looking at the asm: lda $0711; bne @9b84; rts. 
        // This means if A == 0, it returns. If A != 0, it proceeds to @9b84.
        return; 
    }

    // jsr GetTreasurePtr: sets up the base pointer for treasures in WRAM/ROM
    get_treasure_ptr_emu(snes);

    for (uint16_t y = 0; ; y++) {       // ldy #0 / @9b8a
        uint16_t x = read16(ram, 0x3D); // ldx $3d (X is 16-bit)
        
        // Calculate treasure offset: y + ram[0x0FE7]
        uint8_t offset = (uint8_t)(y + ram[0x0FE7]); // clc / adc $0fe7
        ram[0x08FC] = offset;           // sta $08fc

        // jsr CheckTreasureSwitch: checks if the treasure at current offset is active
        // We use the emulated call; it likely returns a result in A
        uint8_t result = (uint8_t)check_treasure_switch_emu(snes);
        
        if (result != 0) {              // cmp #0 / beq @9bbb
            uint16_t x_ptr = read16(ram, 0x3D);
            
            // Access f:MapTriggers1 (likely a label in ROM, accessed via relative or absolute)
            // Based on the asm `lda f:MapTriggers1,x`, this is a table lookup.
            // We assume MapTriggers1 is at a fixed ROM address. 
            // Since we are in C, we access this via the snes->rom or mapped memory.
            // For the purpose of this translation, we assume MapTriggers1 is at a known offset.
            uint8_t trig_lo = snes->rom[0xMapTriggers1 + x_ptr]; // lda f:MapTriggers1,x
            uint8_t trig_hi = snes->rom[0xMapTriggers1 + x_ptr + 1]; // lda f:MapTriggers1+1,x
            
            ram[0x18] = trig_lo;        // sta $18
            ram[0x19] = trig_hi;        // sta $19
            
            // Look up trigger value in a secondary table at $7f5c71
            uint8_t trig_val = ram[0x7F5C71 + trig_lo]; // ldx $18 / lda $7f5c71,x
            if (trig_val == 0x78) {     // cmp #$78 / bne @9bbb
                ram[0x7F5C71 + trig_lo] = 0x77; // lda #$77 / sta $7f5c71,x
            }
        }

        // Update loop counter and index
        uint16_t next_x = read16(ram, 0x3D);
        write16(ram, 0x3D, next_x + 5); // inx5 / stx $3d (inx5 is a macro for 5 iterations of INX)
        
        if (y + 1 == ram[0x0711]) {    // iny / tya / cmp $0711 / beq @9bce
            break;
        }
    }
}

// PITFALLS: 7 (Arithmetic truncation: y + ram[0x0FE7] cast to uint8_t), 
// 8 (Inherited mode: A 8-bit, X/Y 16-bit).
// HELPERS: get_treasure_ptr_emu(snes), check_treasure_switch_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x0711=1, 0x0FE7=1, 0x3D=2
//   output_ram:  0x08FC=1, 0x18=1, 0x19=1, 0x3D=2, 0x7F5C71=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::InitTreasures ($9B:7E)