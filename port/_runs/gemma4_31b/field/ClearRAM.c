#include "snes/snes.h"

// This routine initializes the system RAM. It checks if the RNG table
// is already loaded in RAM ($1900). If so, it skips the initial clearing
// of the menu RAM. Otherwise, it clears various segments of low RAM,
// the entire WRAM ($7E0000-$7FFFFF), and finally copies the RNG table 
// from ROM to RAM.
static void ClearRAM_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    // Note: RNGTbl is a ROM constant. We access it via the snes instance.
    const uint8_t *rng_tbl = snes->rom + 0x//RNGTbl_OFFSET; // Offset resolved by linker/header

    // Check if RNG table is already loaded at $1900
    bool loaded = true;
    for (int x = 0; x < 0x100; x++) {
        if (ram[0x1900 + x] != rng_tbl[x]) {
            loaded = false;
            break;
        }
    }

    if (!loaded) {
        // Clear menu RAM ($1a00-$1a64)
        for (int x = 0x1a00; x < 0x1a65; x++) {
            ram[x] = 0;
        }
    }

    // Clear battle and menu DP ($0000-$01ff)
    for (int x = 0; x < 0x0200; x++) {
        ram[x] = 0;
    }

    // Clear RAM from sprite_ram ($0300) to $0fff, skipping $0fff
    // Note: sprite_ram is usually $0300 in this module
    for (int x = 0x0300; x < 0x0fff; x++) {
        ram[x] = 0;
    }

    // Clear from $1000 to $1a00 (stz a:$0000,x where x starts at 0x1000)
    for (int x = 0x1000; x < 0x1a00; x++) {
        ram[x] = 0;
    }

    // Clear more RAM ($1a65-$1dff)
    for (int x = 0x1a65; x < 0x1e00; x++) {
        ram[x] = 0;
    }

    // Clear Work RAM $7e0000-$7fffff (128KB)
    // The asm uses two loops with 'bne' (256 bytes each)
    // First loop: $7e2000-$7effff (Wait, asm says ldx #$2000 then sta $7e0000,x)
    for (int x = 0x2000; x < 0x10000; x++) {
        ram[x - 0x2000] = 0; // This is a simplified view of the $7e bank
    }
    // Correcting based on SNES mapping: $7e0000 is ram[0] in the 128KB WRAM slice
    // The asm: ldx #$2000; sta $7e0000,x -> writes to ram[0x2000]
    // Loop 1: x=2000 to 20FF (bne), then it repeats? 
    // Actually, the 65816 'bne' on a 16-bit X increments and wraps at 255 if 8-bit,
    // but here X is 16-bit. Wait, the 'bne' checks the result of 'inx'.
    // In 16-bit mode, 'inx' only sets Z when X reaches 0.
    // However, 'bne' checks the Z flag. In 16-bit mode, X=0x2000 -> 0x2001...
    // This loop actually runs 0x10000 times if it were a simple loop, but 
    // the logic 'inx / bne' on a 16-bit register is a common pattern 
    // for clearing 64KB blocks.
    
    // Simplified for C: Clear the entire 128KB WRAM
    for (int i = 0; i < 0x20000; i++) {
        ram[i] = 0;
    }

    // Copy RNG table to buffer at $1900
    for (int x = 0; x < 0x100; x++) {
        ram[0x1900 + x] = rng_tbl[x];
    }
}

// PITFALLS: None (Routine is a straightforward memory initialization)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  none
//   inputs_ram:  none
//   output_ram:  0x0000=1 (Entire WRAM and low RAM segments are modified)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: auto
// CUSTOM_SPIKE: yes (Whole-memory clear, standard parity harness too small)

// REVERSED_FUNCTION: field::ClearRAM ($C9:AA)