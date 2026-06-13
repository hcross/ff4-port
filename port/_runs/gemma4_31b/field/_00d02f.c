#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$D0, DP=0
// Logic:
//   1. Wait for VBlank (delegated)
//   2. Load value from $60, subtract 16, store in hBG2VOFS
//   3. Load value from $61, subtract 0, store in hBG2VOFS
//
// Note: The ASM shows `sta hBG2VOFS` twice. Based on the subtraction 
// sequence and typical SNES offset handling, these are likely 
// contiguous bytes of a 16-bit value or a mistake in the disassembly 
// symbol mapping. We treat hBG2VOFS as a symbol for the target address.
static void _00d02f_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    wait_vblank_short_emu(snes); // jsr WaitVblankShort

    // lda $60 / sec / sbc #16 / sta hBG2VOFS
    // Since hBG2VOFS is not defined in the prompt's RAM map, we use 
    // the likely symbol target or the implied memory location.
    // Given the context of "V-Offset", this is usually a specific hardware 
    // or WRAM register.
    uint8_t val60 = ram[0x60];
    uint8_t res60 = (uint8_t)(val60 - 16); // Pitfall 7: 8-bit truncation
    ram[0x38FD] = res60; // Assuming hBG2VOFS is a placeholder for a specific addr

    // lda $61 / sbc #0 / sta hBG2VOFS
    // Note: SBC #0 with Carry set (from previous SEC) results in val + 1
    uint8_t val61 = ram[0x61];
    uint8_t res61 = (uint8_t)(val61 + 1); 
    ram[0x38FE] = res61; // Assuming the second store targets the next byte
}

// PITFALLS: 7 (8-bit subtraction/addition truncation)
// HELPERS: wait_vblank_short_emu(snes) — delegates WaitVblankShort @ 913C
//
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x60=1, 0x61=1
//   output_ram:  0x38FD=1, 0x38FE=1 (approximate hBG2VOFS)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xD0
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Exact address of hBG2VOFS required for parity)

// REVERSED_FUNCTION: field::_00d02f ($D0:2F)