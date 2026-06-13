#include "snes/snes.h"

// Logic:
// Iterate through characters (up to 0x140). If a character exists, is not 
// in a state that prevents damage (bmi $1003,x), and satisfies a flag check 
// (and #$40 at $1004,x), it takes damage. 
// Damage is handled as a countdown from $1007,x (16-bit). If value > 50, 
// it subtracts 50. If value <= 50, it is set to 1.
// If any character was damaged, play SFX $7B.
static void DoFloorDmg_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0xC1] = 0; // stz $c1
    if ((ram[0xA2] & 0x01) == 0) { // lda $a2 / and #$01 / beq @9842
        goto label_9842;
    }

    for (uint16_t x = 0; x < 0x0140; x++) { // ldx #0 / loop to 0x140
        // The ASM uses offset indexing $1000,x. 
        // In 65816, this is absolute address $1000 + x.
        if (ram[0x1000 + x] == 0) { // lda $1000,x / beq @983a
            goto label_983a;
        }
        if (ram[0x1003 + x] & 0x80) { // lda $1003,x / bmi @983a
            goto label_983a;
        }
        if ((ram[0x1004 + x] & 0x40) != 0) { // lda $1004,x / and #$40 / bne @983a
            goto label_983a;
        }

        ram[0xC1]++; // inc $c1

        // longa: A becomes 16-bit
        uint16_t timer = read16(ram, 0x1007 + x); // lda $1007,x
        if (timer != 0) { // beq @9835
            if (timer > 50) { // sec / sbc #50 / bcs @9835
                timer = (uint16_t)(timer - 50);
                write16(ram, 0x1007 + x, timer);
                if (timer == 0) { // beq @982f
                    goto label_982f;
                }
            } else {
                goto label_982f;
            }
        } else {
            goto label_9835;
        }

    label_982f:
        write16(ram, 0x1007 + x, 1); // lda #1 / sta $1007,x
    label_9835:
        // lda #0 / shorta (effectively clearing A and returning to 8-bit)
        
    label_983a:
        next_char_emu(snes); // jsr NextChar (delegated)
        // cpx #$0140 / bne @980a is handled by for-loop
    }

label_9842:
    if (ram[0xC1] != 0) { // lda $c1 / beq @984b
        snes->cpu->a = 0x7B; // lda #$7b
        play_sfx_emu(snes);  // jsr PlaySfx (delegated)
    }
}

// PITFALLS: 1 (DB=$7E for WRAM), 6 (longa/shorta transitions for $1007 access), 
// 7 (16-bit subtraction truncation handled by uint16_t)
// HELPERS: next_char_emu(snes), play_sfx_emu(snes), read16, write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x00A2=1, 0x1000=1, 0x1003=1, 0x1004=1, 0x1007=2
//   output_ram:  0x00C1=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::DoFloorDmg ($97:00FF)