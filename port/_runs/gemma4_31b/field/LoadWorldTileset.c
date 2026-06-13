#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X/Y 16-bit (xf=0), DB=0x00, DP=0x00
// This routine copies 512 bytes from a source address (pointed to by DP)
// to a fixed destination in WRAM ($7F:4800).
//
// Logic:
//   1. Use DP as a base pointer to read 512 bytes.
//   2. Store bytes into WRAM starting at $7F:4800.
//   3. Clear A (8-bit) before returning.
static void LoadWorldTileset_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // pha / plb sequence sets B register to current A
    // lda a:$0000,y translates to load from (B << 8) | (DP + Y)
    uint16_t source_base = (uint16_t)((cpu->a << 8) | cpu->dp);

    for (uint16_t x = 0; x < 0x0200; x++) {
        // lda a:$0000,y (where y is essentially x)
        // Note: The asm uses 'iny' and 'inx' in parallel.
        uint8_t byte = ram[source_base + x]; 
        
        // sta $7f4800,x
        // $7F4800 is mapped to snes->ram[0x14800] (0x7F0000 is the start of WRAM bank 1, 
        // but LakeSnes/Snesrev mapping usually treats 128KB WRAM as a flat array.
        // $7E:0000 -> 0, $7F:0000 -> 0x10000. So $7F:4800 -> 0x14800)
        ram[0x14800 + x] = byte;
    }

    // lda #0 / pha / plb
    cpu->a = 0;
}

// PITFALLS: 1 (DB/DP context), 6 (Mode A 8-bit vs 16-bit: loop uses 8-bit loads),
// 8 (Inherited mode: assume mf=true, xf=false for field module logic).
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x0000=1 (source pointed to by A/DP)
//   output_ram:  0x14800=1 (destination start)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::LoadWorldTileset ($F6:0D)