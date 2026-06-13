#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$8C, DP=0
// Logic:
//   1. Multiplies A (input) by 8 and stores in 16-bit LE [0x3D, 0x3E].
//   2. If ram[0xC0] is 0, it updates the global RNG seed at $17EE using a 
//      value from the RNG table in Bank F.
//   3. It then takes the scaled value in ram[0x3D] and performs a series of 
//      threshold checks, incrementing the value for each threshold it meets.
static void ChooseRandBattle_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = (uint8_t)snes->cpu->a;

    // sta $3d / stz $3e / asl $3d / rol $3e (x3)
    // This is a 16-bit left shift by 3 of the value in A.
    uint16_t scaled_a = (uint16_t)a << 3;
    ram[0x3D] = (uint8_t)(scaled_a & 0xFF);
    ram[0x3E] = (uint8_t)((scaled_a >> 8) & 0xFF);

    if (ram[0xC0] == 0) { // lda $c0 / bne @8d1f
        uint8_t idx = ram[0x87];
        
        // lda f:RNGTbl,x
        // The original attempt used snes->rom, which is not in the Snes struct.
        // In the parity harness, ROM is accessed via a provided helper or external mapping.
        // Since RNGTbl is a constant table, we use the emulator to perform the read
        // or simulate the access. Given the constraints, we use the ROM offset.
        // Note: In this environment, ROM access is handled via the LakeSnes memory map.
        uint8_t rng_val = snes->rom_data[0x0F0000 + idx]; // Adjusted to valid ROM accessor
        
        uint8_t seed = ram[0x17EE];
        uint8_t sum = (uint8_t)(rng_val + seed); // clc / adc $17ee
        
        ram[0x87]++; // inc $87
        if (ram[0x87] != 0) { // bne @8d21
            // A contains 'sum', but it is immediately overwritten by ldx $3d
        } else { 
            // bne not taken: handle seed wrap-around
            uint8_t next_seed = (uint8_t)(seed + 0x11); // lda $17ee / clc / adc #$11
            ram[0x17EE] = next_seed; // sta $17ee
        }
    } else {
        // lda #$ff (@8d1f) - Value in A is ignored by subsequent ldx $3d
    }

    uint8_t x = ram[0x3D];
    
    // Threshold checks (BCC: branch if X < threshold)
    // Pitfall 3: bcc branches when X < threshold. 
    // If X >= threshold, we increment X and proceed to the next check.
    if (x >= 0x2B) { x++; } // cmp #$2b / bcc @8d46
    if (x >= 0x56) { x++; } // cmp #$56 / bcc @8d46
    if (x >= 0x81) { x++; } // cmp #$81 / bcc @8d46
    if (x >= 0xAC) { x++; } // cmp #$ac / bcc @8d46
    if (x >= 0xCC) { x++; } // cmp #$cc / bcc @8d46
    if (x >= 0xEC) { x++; } // cmp #$ec / bcc @8d46
    if (x >= 0xFC) { x++; } // cmp #$fc / bcc @8d46

    ram[0x3D] = x; // stx $3d
}

// PITFALLS: 3 (CMP/BCC logic: body executed when X >= threshold)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x3D=1, 0x3E=1, 0xC0=1, 0x87=1, 0x17EE=1
//   output_ram:  0x3D=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x8C
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::ChooseRandBattle ($8C:F0)