#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=0x94 (assuming module bank), DP=0
// Logic:
//   1. Copies 20 bytes from f:CharProp to ram[$1000]
//   2. Copies 3 bytes from f:CharProp[20..22] to ram[$102D]
//   3. Copies 9 bytes from f:CharProp[23..31] to ram[$1037]
//   4. Calls InitMainCharEquip and UpdateEquip_ext
static void InitCharProp_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // f:CharProp is in the ROM/Bank mapping. 
    // Based on the ASM, we access it via the current bank (0x94).
    // In the emulator harness, this typically maps to a specific ROM area.
    const uint8_t *char_prop = &snes->rom[0x9400 + 0x0000]; // Adjust offset if f:CharProp is defined otherwise

    // First loop: @94f4
    for (uint16_t x = 0, y = 0; y < 20; x++, y++) {
        ram[0x1000 + y] = char_prop[x];
    }

    // Second loop: @9505
    // x is now 20
    for (uint16_t x = 20, y = 0; y < 3; x++, y++) {
        ram[0x102D + y] = char_prop[x];
    }

    // Third loop: @9516
    // x is now 23
    for (uint16_t x = 23, y = 0; y < 9; x++, y++) {
        ram[0x1037 + y] = char_prop[x];
    }

    // Final sequence
    ram[0x3D] = 0;                           // ldy #0 / sty $3d
    init_main_char_equip_emu(snes);          // jsr InitMainCharEquip
    
    snes->cpu->a = 0;                        // lda #0
    update_equip_ext_emu(snes);              // jsl UpdateEquip_ext
}

// PITFALLS: 1 (Direct Page/Bank access), 6 (A 8-bit vs 16-bit - routine uses 8-bit LDA/STA)
// HELPERS: init_main_char_equip_emu(snes), update_equip_ext_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none (reads from ROM f:CharProp)
//   output_ram:  0x1000=1, 0x102D=1, 0x1037=1, 0x3D=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x94
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::InitCharProp ($94:EE)