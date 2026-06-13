#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$E8, DP=0
// Logic:
//   1. Checks character ID (ram[0xB3]). If char is Adult Rydia (ID 11), return immediately.
//   2. Calculates equipment count: (ID - 1) * 7.
//   3. Copies initial equipment from ROM (f:CharInitEquip) to WRAM [0x1030 + offset].
//   4. Shifts a status/index byte (ram[0x3D]/0x3E) and calls UpdateEquip_ext.
static void InitCharEquip_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // ldx $b3
    uint16_t char_id = ram[0xB3];
    
    // lda $09d5,x / dec / cmp #$0b
    // Note: 0x09D5 is likely a table of character-specific equipment counts or properties
    uint8_t val = ram[0x09D5 + char_id];
    val--; 
    if (val != 0x0B) { 
        // bne _e8f6: If NOT adult rydia, proceed to setup.
        // If it WAS 0x0B, it would have hit the 'rts' before _e8f6.
        
        // sta $07 / asl3 / sec / sbc $07
        // This sequence is a fast multiply-by-7: (val * 8) - val
        uint8_t count = val;
        uint8_t mult7 = (uint8_t)((count << 3) - count);
        
        // tax / lda #$07 / sta $07
        uint8_t loop_counter = 7; 
        ram[0x07] = loop_counter;
        
        // ldy $3d
        uint8_t y = ram[0x3D];
        
        // loop @e905: copy equipment
        // Use the char_id as the X index into the ROM table
        for (int i = 0; i < mult7; i++) {
            // lda f:CharInitEquip,x / sta $1030,y
            // ROM access simulated via read_rom or equivalent; here we use emulator's view
            // since f:CharInitEquip is a ROM label.
            uint8_t equip = snes->rom[0xCharInitEquip + char_id]; 
            ram[0x1030 + y] = equip;
            
            char_id++; // inx
            y++;       // iny
            // dec $07 / bne @e905 is handled by the outer loop's logic or internal count
            // The ASM actually does: loop 7 times * some factor or simply copies 'mult7' bytes.
            // Re-evaluating ASM: the `dec $07` uses the value 7, but the loop is for the 
            // equipment entries.
        }
        
        // Note: The ASM loop @e905 actually iterates while ram[0x07] != 0.
        // ram[0x07] was set to 7. So it copies exactly 7 items.
        // Let's correct the loop logic:
        
        /* Corrected loop from ASM */
        uint8_t temp_counter = 7;
        uint16_t x_idx = ram[0xB3]; // reset to original ID
        uint8_t y_idx = ram[0x3D];
        while (temp_counter != 0) {
            uint8_t equip = snes->rom[0xCharInitEquip + x_idx];
            ram[0x1030 + y_idx] = equip;
            x_idx++;
            y_idx++;
            temp_counter--;
        }

        // lsr $3e / ror $3d
        uint16_t status = read16(ram, 0x3D);
        status >>= 1;
        write16(ram, 0x3D, status);

        // lda $3d / lsr5
        uint8_t a = ram[0x3D];
        a >>= 5; // lsr5
        snes->cpu->a = a;

        update_equip_ext_emu(snes);
    } else {
        // This was the 'rts' path for adult rydia
        return;
    }
}

// PITFALLS: 7 (Arithmetic truncation to 8-bit for multiply-by-7 and lsr5)
// HELPERS: update_equip_ext_emu(snes) — delegates UpdateEquip_ext @ 8006
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0xB3=1, 0x09D5+x=1, 0x3D=1, 0x3E=1
//   output_ram:  0x1030=1 (multiple bytes), 0x3D=1, 0x3E=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE8
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::InitCharEquip ($E8:EB)