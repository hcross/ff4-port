#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$99 (assumed based on bank), DP=0
// This routine adds a constant amount of gil (split across 3 bytes) to the player's 
// gil total and clamps the result to a maximum value (0x7F9698).
//
// Logic:
//   1. Add bytes at $30, $31, $32 to ram[$16A0], ram[$16A1], ram[$16A2] with carry.
//   2. If result > 0x7F9698, cap gil at 0x7F9698.
static void GiveGil_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t carry = 0;

    // Add Gil: 3-byte BCD-like or raw addition sequence
    // Note: The asm uses absolute $30, $31, $32. 
    // In field module, DB is usually $99 or the bank the code resides in.
    // $30 is likely a constant value in the ROM bank.
    
    uint8_t a0 = ram[0x16A0];
    uint8_t add0 = snes->ram[0x30]; // Assuming DB is used for these reads
    uint16_t res0 = (uint16_t)a0 + add0 + carry;
    ram[0x16A0] = (uint8_t)res0;     // Pitfall 7: truncate to 8-bit
    carry = (res0 > 0xFF);

    uint8_t a1 = ram[0x16A1];
    uint8_t add1 = snes->ram[0x31];
    uint16_t res1 = (uint16_t)a1 + add1 + carry;
    ram[0x16A1] = (uint8_t)res1;     // Pitfall 7: truncate to 8-bit
    carry = (res1 > 0xFF);

    uint8_t a2 = ram[0x16A2];
    uint8_t add2 = snes->ram[0x32];
    uint16_t res2 = (uint16_t)a2 + add2 + carry;
    ram[0x16A2] = (uint8_t)res2;     // Pitfall 7: truncate to 8-bit

    // Clamp check: if (Gil > 0x7F9698)
    // Checks are done in reverse (High byte to Low byte)
    if (ram[0x16A2] < 0x98) {        // cmp #$98 / bcc @9a35
        return;
    }
    if (ram[0x16A1] < 0x96) {        // cmp #$96 / bcc @9a35
        return;
    }
    if (ram[0x16A0] < 0x7F) {        // cmp #$7f / bcc @9a35
        return;
    }

    // Cap at 0x7F9698
    ram[0x16A0] = 0x7F;
    ram[0x16A1] = 0x96;
    ram[0x16A2] = 0x98;
}

// PITFALLS: 7 (Arithmetic truncation in 8-bit mode: used uint16_t intermediate 
// and cast to uint8_t to simulate 65816 carry/overflow behavior).
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x16A0=1, 0x16A1=1, 0x16A2=1, 0x30=1, 0x31=1, 0x32=1
//   output_ram:  0x16A0=1, 0x16A1=1, 0x16A2=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x99
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GiveGil ($99:FB)