#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=0 (Direct Page addressing), DP=0
// Purpose: Iterates through characters to apply poison damage if they are poisoned.
// If any character took damage, plays the poison SFX.
static void DoPoisonDmg_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xC2] = 0; // stz $c2
    if (ram[0x1704] == 0) return; // lda $1704 / bne @97fe (Return if poison flag is 0)

    for (uint16_t x = 0; x != 0x0140; x++) { // ldx #0 / loop / cpx #$0140
        // Offset calculation: $1000 + x (effectively indexed access to character data)
        // Since X is 16-bit, this access spans beyond page 0.
        uint8_t char_active = ram[0x1000 + x];
        if (char_active == 0) goto next_char_block; // beq @97e9

        uint8_t is_poisoned = ram[0x1003 + x] & 0x01; // lda $1003,x / and #$01
        if (is_poisoned == 0) goto next_char_block; // beq @97e9

        ram[0xC2]++; // inc $c2 (Poison hit counter)

        // Process poison timer (16-bit)
        uint16_t timer = read16(ram, 0x1007 + x); // longa / lda $1007,x
        if (timer != 0) { // beq @97e4
            timer--; // sec / sbc #1
            if (timer < 1) { // cmp #1 / bcs @97e4 (Inverted: if timer < 1, set to 1)
                timer = 1; // lda #1 / sta $1007,x
            }
            write16(ram, 0x1007 + x, timer); // sta $1007,x
        }
        // shorta occurs here; A is reset to 0 but effectively discarded

    next_char_block:
        next_char_emu(snes); // jsr NextChar
    }

    if (ram[0xC2] == 0) return; // lda $c2 / beq @97fe
    if (ram[0xB1] != 0) return; // lda $b1 / bne @97fe

    play_sfx_emu(snes, 0x7A); // lda #$7a / jsr PlaySfx (Assuming PlaySfx reads A)
}

// PITFALLS: 3 (CMP/BCS inversion: bcs branches when A>=1, so if A<1 we set it to 1), 
// 6 (longa/shorta: poison timer $1007 is 16-bit), 7 (8-bit truncation on counter $c2)
// HELPERS: next_char_emu(snes), play_sfx_emu(snes, val)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1704=1, 0x1000=1, 0x1003=1, 0x1007=2, 0x0B1=1
//   output_ram:  0x00C2=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::DoPoisonDmg ($97:B3)