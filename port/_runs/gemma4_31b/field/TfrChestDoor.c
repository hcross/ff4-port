#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0x00, DP=0
// This routine handles the transfer of a chest door's position to hardware 
// registers and updates the door's coordinates in RAM.
// It checks a trigger flag at $D4; if 0, it returns immediately.
static void TfrChestDoor_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    if (ram[0xD4] == 0) { // lda $d4 / bne @c7ea
        return;
    }

    ram[0xD4] = 0;        // stz $d4
    
    // Hardware register writes ($2115 is usually a control register, 
    // $2116/$2118 are often used for sprite/object coordinates)
    snes->io[0x15] = 0x80; // lda #$80 / sta $2115

    uint16_t x = read16(ram, 0x06FE); // ldx $06fe
    snes->io[0x16] = (uint16_t)x;    // stx $2116 (likely writing to low/high bytes)
    
    x = read16(ram, 0x0700); // ldx $0700
    snes->io[0x18] = (uint16_t)x;    // stx $2118

    x = read16(ram, 0x0702); // ldx $0702
    snes->io[0x18] = (uint16_t)x;    // stx $2118 (overwrites previous write)

    // 16-bit addition for coordinate update: $06FE += 0x20
    uint8_t lo = ram[0x06FE];
    uint8_t carry = 0;
    uint8_t res_lo = (uint8_t)(lo + 0x20); // clc / adc #$20 / sta $06fe
    if (res_lo < lo) carry = 1;            // Simulate C flag for 8-bit add
    ram[0x06FE] = res_lo;

    uint8_t hi = ram[0x06FF];
    ram[0x06FF] = (uint8_t)(hi + carry);   // adc #$00 / sta $06ff

    // Final hardware updates
    uint16_t final_x = read16(ram, 0x06FE);
    snes->io[0x16] = (uint16_t)final_x;    // ldx $06fe / stx $2116
    
    x = read16(ram, 0x0704);
    snes->io[0x18] = (uint16_t)x;    // ldx $0704 / stx $2118
    
    x = read16(ram, 0x0706);
    snes->io[0x18] = (uint16_t)x;    // ldx $0706 / stx $2118
}

// PITFALLS: 7 (Arithmetic truncation: manually simulated 8-bit carry 
// across $06FE and $06FF to match 65816 ADC behavior).
// HELPERS: read16 (little-endian WRAM access).
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00D4=1, 0x06FE=2, 0x0700=2, 0x0702=2, 0x0704=2, 0x0706=2
//   output_ram: 0x00D4=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (writes to IO registers $2115-$2118)
// REVERSED_FUNCTION: field::TfrChestDoor ($C7:E5)