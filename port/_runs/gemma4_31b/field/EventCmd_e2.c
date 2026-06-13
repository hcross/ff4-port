#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$EA, DP=0
// Logic:
//   1. Read value from $09D5 + X.
//   2. Perform a multiplication by 7 (asl3 then (val << 1) + val).
//   3. Store result in $18 (temp).
//   4. Search for the first zero-byte in the table at $1560 using the result as offset.
//   5. Replace that zero-byte with the next event byte.
//   6. Jump to WaitVblankEvent.
static void EventCmd_e2_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *cpu = snes->cpu;

    // inx / lda $09d5,x (8-bit)
    cpu->x++;
    uint8_t val = ram[0x09D5 + cpu->x];

    // longa / asl3 (val << 3)
    uint16_t a16 = (uint16_t)val << 3;

    // sta $18 (temp storage)
    write16(ram, 0x18, a16);

    // asl / clc / adc $18  => (a16 * 2) + a16 = a16 * 3
    // Total result: (val << 3) * 3 = val * 24? No, look closer:
    // asl is a16 * 2. Then adc $18 is + (a16). 
    // Result is a16 * 2 + a16 = a16 * 3.
    // Since a16 was (val << 3), result is (val * 8) * 3 = val * 24.
    uint16_t result = (a16 << 1) + a16;
    
    // tay / lda #$0000 / shorta
    uint16_t y = result;
    // shorta: A becomes 8-bit, but Y remains 16-bit (xf=0)

    // Table search loop @ea75
    while (1) {
        uint8_t table_val = ram[0x1560 + y]; // lda $1560,y
        if (table_val == 0) break;           // beq @ea7e
        y++;                                 // iny
    }

    // GetNextEventByte (delegated)
    uint8_t next_byte = (uint8_t)get_next_event_byte_emu(snes);
    
    // sta $1560,y
    ram[0x1560 + y] = next_byte;

    // jmp WaitVblankEvent (delegated as a function call)
    wait_vblank_event_emu(snes);
}

// PITFALLS: 6 (Mode A transition: 8-bit -> 16-bit -> 8-bit), 
// 8 (Inherited mode: assumes mf=true, xf=false on entry)
// HELPERS: get_next_event_byte_emu(snes), wait_vblank_event_emu(snes)
// CONTRACT:
//   inputs_reg:  x=16, y=none, a=none
//   inputs_ram:  0x09D5+X=1, 0x1560[offset]=1
//   output_ram:  0x1560[offset]=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xEA
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::EventCmd_e2 ($EA:60)