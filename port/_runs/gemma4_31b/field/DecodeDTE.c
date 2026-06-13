#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$B2 (implied by ROM bank), DP=0
// Logic:
//   The routine calculates a table index based on the input A, 
//   fetches two characters from the DTETbl table, and writes them 
//   to RAM starting at $0774 + Y.
//   Finally, it updates RAM $3D with the final Y value and jumps to _b288.
static void DecodeDTE_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // sec / sbc #$80 / asl
    // In 8-bit mode: A = (A + 1) - 0x80, then A << 1.
    // Pitfall 7: Ensure 8-bit truncation.
    uint8_t a_val = (uint8_t)(cpu->a + 1 - 0x80);
    uint8_t index = (uint8_t)(a_val << 1);

    // The table DTETbl is in ROM. We use the emulator to read the ROM 
    // values or a simulated read. Since we don't have the ROM array 
    // mapped in C, we must handle the table access via the emulator 
    // or a helper. However, the target is to translate the logic.
    // To maintain parity, we emulate the table lookup.
    
    // Setup registers for the jump/delegation to _b288
    // but first we must perform the writes to RAM.
    
    // We need to fetch from DTETbl (f:DTETbl). 
    // Since DTETbl is a ROM address, we simulate the read.
    // For this translation, we use a helper to get the table bytes.
    
    uint8_t first_char = read_rom_byte(snes, 0xDTETbl_ADDR + index); 
    uint8_t second_char = read_rom_byte(snes, 0xDTETbl_ADDR + index + 1);

    uint16_t y_val = cpu->y;
    ram[0x0774 + y_val] = first_char;
    y_val++;
    ram[0x0774 + y_val] = second_char;
    y_val++;

    ram[0x3D] = (uint8_t)y_val; // sty $3d (DP=0)
    cpu->y = y_val;

    // jmp _b288: This is a tail-call to another routine.
    _b288_emu(snes);
}

// PITFALLS: 7 (8-bit arithmetic truncation for sbc/asl)
// HELPERS: _b288_emu(snes) — delegates the jump target at $B2:88
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=16
//   inputs_ram:  none
//   output_ram:  0x0774=<1>, 0x0775=<1>, 0x3D=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xB2
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::DecodeDTE ($B2:DC)