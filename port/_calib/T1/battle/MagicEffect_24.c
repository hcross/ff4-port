#include "snes/snes.h"

// Entry mode: mf=true (A 8-bit), xf=false (X/Y 16-bit), DB=0xDD, DP=0
// Logic:
//   Calculates (ram[0x2709..270A] - ram[0x2707..2708]) as 16-bit signed subtraction.
//   Writes result to 0xA4-0xA5.
//   Sets the high bit of the high byte (0xA5) to 1 (forcing negative/flagged state).
//   Jumps to SetMagicStatus.
static void MagicEffect_24_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // longa: A 16-bit
    // sec: Carry = 1
    // lda $2709 / sbc $2707
    uint16_t val1 = read16(ram, 0x2709);
    uint16_t val2 = read16(ram, 0x2707);
    uint16_t result = (uint16_t)(val1 - val2);

    // sta $a4 (16-bit write)
    write16(ram, 0xA4, result);

    // shorta0: A = D (0), then 8-bit mode
    // However, the code immediately performs:
    // lda $a5 / ora #$80 / sta $a5
    uint8_t high_byte = ram[0xA5];
    high_byte |= 0x80; // ora #$80
    ram[0xA5] = high_byte;

    // jmp SetMagicStatus
    set_magic_status_emu(snes);
}

// PITFALLS: 6 (Mixed 16-bit/8-bit A mode: handle 16-bit read/write for 0x270x
// and 0xA4, then switch to 8-bit byte manipulation for 0xA5).
// HELPERS: set_magic_status_emu(snes) — delegates SetMagicStatus @ $D505
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2707=2, 0x2709=2
//   output_ram:  0xA4=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xDD
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: battle::MagicEffect_24 ($DD:06)