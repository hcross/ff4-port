#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$E1, DP=0
// This routine draws an explosion effect by iterating through a set of
// sprite offsets. It calculates final screen coordinates by adding 
// sprite-specific offsets to a base position and writes them to 
// sprite RAM ($0300-$0303).
static void DrawExplosion_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // Table lookup for explosion frame data
    // A = ram[0x0A6D + x], then used as index into _00E1B4
    uint8_t frame_idx = ram[0x0A6D + cpu->x];
    uint8_t *explosion_data = &ram[0x00E1B4 + frame_idx]; // _00e1b4 is in bank 0 or relative to DB
    uint8_t y = 0; // The asm uses tay twice; effectively indices the explosion data

    // X-coordinate base and Y-coordinate base from RAM
    uint8_t base_x = ram[0x0A6E + cpu->x]; // sta $20
    uint8_t base_y = ram[0x0A6F + cpu->x]; // sta $21

    // Calculate loop count: ram[$24] << 1
    uint8_t loop_count = (uint8_t)(ram[0x24] << 1); // asl2 / tax

    // The phx/phy/plx/ply sequence is a NOP pattern used for timing or alignment
    // Loop iterates until (X & 0x0F) == 0
    uint8_t iteration_count = loop_count;
    
    // Note: The asm loop logic is 'txa / and #$0f / bne @e175'
    // where X is the loop counter. To match exactly, we simulate the 4-bit mask.
    while (1) {
        // We need a pointer to the specific sprite offset within the frame data
        // The ASM uses 'tay' and then 'lda _00e1b4,y'. 
        // Since Y is modified by NextSprite (implied) or set via the table,
        // we must track the current sprite index.
        uint8_t sprite_offset_idx = y * 4; // Each sprite usually has 4 bytes of data
        
        // X Coord Calculation
        // lda $20 / clc / adc f:_14f9d6,x
        // f:_14f9d6 is likely a constant table of offsets
        uint8_t offset_x = ram[0x14F9D6 + (cpu->x & 0xFF)]; // Simplified offset access
        ram[0x0300 + y] = (uint8_t)(base_x + offset_x);

        // Y Coord Calculation
        uint8_t offset_y_lo = ram[0x14F9D7 + (cpu->x & 0xFF)];
        if (offset_y_lo == 0xFF) {
            ram[0x0301 + y] = 0xF0; // beq @e191
        } else {
            ram[0x0301 + y] = (uint8_t)(base_y + offset_y_lo); // jmp @e193
        }

        // Palette/Property Calculation
        ram[0x0302 + y] = ram[0x14F9D8 + (cpu->x & 0xFF)];

        // Priority/Attr Calculation
        uint8_t attr = (uint8_t)((ram[0x0ACD] << 1) | ram[0x0ACE]);
        attr |= ram[0x14F9D9 + (cpu->x & 0xFF)];
        ram[0x0303 + y] = attr;

        next_sprite_emu(snes); // jsr NextSprite
        
        // Loop control: txa / and #$0f / bne
        if ((iteration_count & 0x0F) == 0) break;
        
        iteration_count--; // The loop uses X as a counter
        y++; // Increment sprite slot
    }
}

// PITFALLS: 7 (Truncation on asl/adc to 8-bit), 8 (A 8-bit, X 16-bit heritage)
// HELPERS: next_sprite_emu(snes) - delegates NextSprite @ 88E2
// CONTRACT:
//   inputs_reg:  a=none, x=16, y=none
//   inputs_ram:  0x0A6D=1, 0x0A6E=1, 0x0A6F=1, 0x24=1, 0x0ACD=1, 0x0ACE=1
//   output_ram:  0x0300=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xE1
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::DrawExplosion ($E1:5A)