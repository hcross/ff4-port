#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=0x8A, DP=0
// This routine copies a specific set of graphics data (Cave Water Gfx) 
// from the ROM/FIRAM to the VRAM buffer at $5800.
// It implements a nested loop that copies 16 bytes, then inserts a null 
// byte, repeating until 0x100 bytes have been written to the destination.
static void LoadCaveWaterGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    
    // @CaveWaterGfx := MapGfx_000e + 67 * 24
    // MapGfx_000e is at 0x000E in the graphics bank.
    // Offset = 0x000E + (67 * 24) = 14 + 1608 = 1622 (0x0656)
    // The 'f:' prefix indicates FIRAM/ROM mapping.
    // In the context of the snesrev/ff4 project, we treat these as ROM offsets.
    uint32_t src_addr = 0x0656; 
    
    uint16_t x = 0;
    uint16_t y = 0;

    // The routine performs a complex loop.
    // Outer loop: while (y != 0x100)
    while (y != 0x100) {
        // First inner loop: copy 16 bytes
        do {
            ram[0x5800 + y] = snes->rom[src_addr + x];
            x++;
            y++;
        } while ((y & 0x0F) != 0);

        // @8a76: Sequence of copy, then zero-fill
        do {
            ram[0x5800 + y] = snes->rom[src_addr + x];
            x++;
            y++;
            
            ram[0x5800 + y] = 0; // sta $5800,y (where y was incremented)
            y++;
        } while ((y & 0x0F) != 0);

        // cpy #$0100 / bne @8a68
        // If y hasn't reached 0x100, jump back to the first inner loop.
        if (y != 0x100) {
            continue;
        }
    }
}

// PITFALLS: 6 (Mode A is 8-bit based on lda #$7f and the behavior of 
// the loop counters), 8 (Inherited mf=true for field module).
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x5800=256
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x8A
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LoadCaveWaterGfx ($8A:5E)