#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$DF, DP=0
// Logic:
// This routine iterates through a set of vehicle sprites based on a 
// vehicle type index. It calculates sprite offsets using VehicleSpriteTbl,
// applies positional adjustments from RAM, and writes the resulting 
// data to the sprite buffer at $0300,Y.
static void _00dfc4_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // Initial calculation of the index offset into VehicleSpriteTbl
    // a = ram[0x7A] & 0x02; shifted left by 3 (asl3 macro/sequence)
    uint8_t type_bit = (ram[0x7A] & 0x02);
    uint8_t a = (uint8_t)(type_bit << 3); // Pitfall 7: truncation to 8-bit
    
    // clc / adc $92 / tax
    uint8_t x_idx = (uint8_t)(a + ram[0x92]);
    
    // The loop iterates until (x_idx & 0x0F) becomes 0
    do {
        // lda f:VehicleSpriteTbl,x / clc / adc $0c / sta $0300,y
        // Assuming VehicleSpriteTbl is a ROM table accessed via a helper or direct mapping
        // Here we emulate the ROM access as a read from a conceptual table
        uint8_t sprite_val0 = snes->rom[VEHICLE_SPRITE_TBL + x_idx];
        ram[0x0300 + (snes->cpu->y & 0xFF)] = (uint8_t)(sprite_val0 + ram[0x0C]);

        // lda $0d / adc #$00 / and #$01 / beq @dfe7
        uint8_t check_val = ram[0x0D];
        if ((check_val & 0x01) != 0) {
            snes->cpu->a = 0x00;
            set_sprite_msb_emu(snes); // jsl SetSpriteMSB
        }

        // lda f:VehicleSpriteTbl+1,x / clc / adc $0e / sec / sbc $ad / clc / adc #$08 / sta $0301,y
        uint8_t sprite_val1 = snes->rom[VEHICLE_SPRITE_TBL + 1 + x_idx];
        int8_t val1 = (int8_t)(sprite_val1 + ram[0x0E]);
        val1 = (int8_t)(val1 - ram[0xAD]);
        ram[0x0301 + (snes->cpu->y & 0xFF)] = (uint8_t)(val1 + 0x08);

        // lda f:VehicleSpriteTbl+2,x / clc / adc $8f / sta $0302,y
        uint8_t sprite_val2 = snes->rom[VEHICLE_SPRITE_TBL + 2 + x_idx];
        ram[0x0302 + (snes->cpu->y & 0xFF)] = (uint8_t)(sprite_val2 + ram[0x8F]);

        // lda f:VehicleSpriteTbl+3,x / ora $91 / sta $0303,y
        uint8_t sprite_val3 = snes->rom[VEHICLE_SPRITE_TBL + 3 + x_idx];
        ram[0x0303 + (snes->cpu->y & 0xFF)] = (uint8_t)(sprite_val3 | ram[0x91]);

        next_sprite_emu(snes); // jsr NextSprite

        // txa / and #$0f / bne @dfcf
        if ((x_idx & 0x0F) == 0) break;
        
    } while (1);
}

// PITFALLS: 7 (Arithmetic truncation to 8-bit for A register)
// HELPERS: set_sprite_msb_emu(snes), next_sprite_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=8bits
//   inputs_ram:  0x7A=1, 0x92=1, 0x0C=1, 0x0D=1, 0x0E=1, 0xAD=1, 0x8F=1, 0x91=1
//   output_ram:  0x0300=1, 0x0301=1, 0x0302=1, 0x0303=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xDF
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00dfc4 ($DF:C4)