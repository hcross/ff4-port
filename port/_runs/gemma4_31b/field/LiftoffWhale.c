#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$A3, DP=0
// Logic:
// 1. Initialize counters $7A and $B9 to 0.
// 2. Loop (Animate4): While $79 < 0x20, set airship speed to ($79 >> 1), increment $79.
// 3. Transition: Once $79 >= 0x20, increment $B9 twice, set $AD = $B9 + 0x10.
// 4. Zoom: Call UpdateZoomPal(Snes, $B9 >> 1).
// 5. Loop (Second phase): Increment $79. If $79 != 0x30, repeat Animate4 loop logic.
// 6. Moon Check: If $06C3 is non-zero, clear it and branch to EarthToMoon or MoonToEarth based on $1700.
static void LiftoffWhale_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x7A] = 0;
    ram[0xB9] = 0;

    do {
        animate4_emu(snes);
        uint8_t val79 = ram[0x79];
        if (val79 < 0x20) {         // cmp #$20 / bcs @a33a (Pitfall 3)
            ram[0x06FD] = (uint8_t)(val79 >> 1); // lsr / sta $06fd (Pitfall 7)
            ram[0x79]++;
            continue;               // jmp @a328
        }

        // @a33a
        ram[0xB9] += 2;             // inc $b9 / inc $b9
        uint8_t b9 = ram[0xB9];
        ram[0xAD] = (uint8_t)(b9 + 0x10); // clc / adc #$10 / sta $ad (Pitfall 7)
        
        // UpdateZoomPal takes B9 >> 1 as input (likely passed via register/RAM in emulator)
        snes->cpu->a = (uint8_t)(b9 >> 1); // lsr A
        update_zoom_pal_emu(snes);

        ram[0x79]++;
        if (ram[0x79] != 0x30) {    // cmp #$30 / bne @a328
            continue;               // jmp @a328
        }

        if (ram[0x06C3] == 0) {     // lda $06c3 / bne @a35a
            return;
        }

        // @a35a
        ram[0x06C3] = 0;
        if (ram[0x1700] == 0) {     // lda $1700 / beq EarthToMoon
            earth_to_moon_emu(snes);
        } else {
            moon_to_earth_emu(snes);
        }
        return;

    } while (1);
}

// PITFALLS: 3 (CMP/BCS inversion), 7 (Truncation of 8-bit arithmetic/shifts)
// HELPERS: animate4_emu(snes), update_zoom_pal_emu(snes), earth_to_moon_emu(snes), moon_to_earth_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x79=1, 0x06C3=1, 0x1700=1
//   output_ram:  0x7A=1, 0xB9=1, 0x06FD=1, 0xAD=1, 0x06C3=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xA3
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LiftoffWhale ($A3:24)