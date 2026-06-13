#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$A5, DP=0
// Logic: This routine performs two sequences of VRAM/DMA-style writes to 
// hardware registers $2115, $2116, and $2118.
// It writes blocks of data in 4-byte chunks (determined by `and #$03` loop)
// and increments the address by $80 until a 16-bit counter (ram[0x3D-0x3E]) 
// reaches $02.
static void _00a5ac_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // First Block
    ram[0x07] = 0x70;
    ram[0x2115] = 0;
    write16(ram, 0x3D, 0x0000);

    do {
        uint16_t addr = read16(ram, 0x3D);
        write16(ram, 0x2116, addr);

        // Inner loop: write 4 bytes to $2118
        do {
            ram[0x2118] = ram[0x07];
            ram[0x07]++;
            // Pitfall 7: 8-bit truncation for AND
            // Logic: repeat until (ram[0x07] & 0x03) == 0
        } while ((ram[0x07] & 0x03) != 0);

        // Increment address by $80
        uint16_t current_addr = read16(ram, 0x3D);
        uint16_t next_addr = current_addr + 0x80;
        write16(ram, 0x3D, next_addr);

        // The ASM does a manual 16-bit addition for the comparison counter
        // lda $3e / adc #0 / sta $3e / cmp #02
        // This effectively checks the high byte of the incremented address
        // since $3D is the low byte and $3E is the high byte.
        if (ram[0x3E] == 0x02) break;

    } while (1); // bne @a5b8

    // Second Block
    ram[0x07] = 0x80;
    ram[0x2115] = 0;
    write16(ram, 0x3D, 0x0040);

    do {
        uint16_t addr = read16(ram, 0x3D);
        write16(ram, 0x2116, addr);

        // Inner loop: write 4 bytes to $2118
        do {
            ram[0x2118] = ram[0x07];
            ram[0x07]++;
        } while ((ram[0x07] & 0x03) != 0);

        uint16_t current_addr = read16(ram, 0x3D);
        uint16_t next_addr = current_addr + 0x80;
        write16(ram, 0x3D, next_addr);

        if (ram[0x3E] == 0x02) break;

    } while (1); // bne @a5e7
}

// PITFALLS: 1 (DB=$A5 used for hardware register access), 7 (8-bit truncation 
// for the AND mask check)
// HELPERS: read16, write16
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xA5
//   entry_flags: z=auto, n=auto
// CUSTOM_SPIKE: yes (Multiple hardware register writes; verify against VRAM state)

// REVERSED_FUNCTION: field::_00a5ac ($A5:AC)