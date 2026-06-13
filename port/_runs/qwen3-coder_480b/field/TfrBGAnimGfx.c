#include "snes/snes.h"

// Transfer background animation graphics based on state in $1700 and $7A.
// Performs DMA transfer to hardware registers when conditions are met.
static void TfrBGAnimGfx_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = ram[0x1700];              // lda $1700
    if (a != 3) return;                   // cmp #3 / bne @cb6c
    a = ram[0x7A];                        // lda $7a
    if ((a & 0x06) != 0) return;          // and #$06 / beq @cb6d → skip if not zero

    // Entry condition satisfied, proceed with transfer setup
    ram[0x12] = (a & 0x18);               // and #$18 / sta $12
    ram[0x13] = 0;                        // stz $13

    // Switch to 16-bit A mode for address arithmetic
    // asl $12 four times = shift left by 4 = multiply by 16
    uint16_t addr = (uint16_t)(ram[0x12]) << 4;
    addr += 0x5000;                       // adc #$5000
    write16(ram, 0x12, addr);             // sta $12

    // DMA transfer setup
    ram[0x2115] = 0x80;                   // sta $2115
    ram[0x420B] = 0;                      // stz $420b
    ram[0x4300] = 1;                      // sta $4300
    ram[0x4301] = 0x18;                   // sta $4301
    write16(ram, 0x2116, 0x1200);         // stx $2116
    ram[0x4304] = 0x7F;                   // sta $4304
    write16(ram, 0x2116, 0x1200);         // stx $2116 again

    // Loop 4 times to transfer 4 blocks
    for (int i = 0; i < 4; i++) {
        write16(ram, 0x4302, read16(ram, 0x12)); // ldx $12 / stx $4302
        write16(ram, 0x4305, 0x0080);            // ldx #$0080 / stx $4305
        ram[0x420B] = 1;                         // sta $420b
        ram[0x13] += 2;                          // lda $13 / clc / adc #$02 / sta $13
    }
}

// PITFALLS: 6 (mode A switching), 7 (16-bit shifts), 10 (goto label workaround)
// HELPERS: none
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x1700=1, 0x7A=1
//   output_ram:  none
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::TfrBGAnimGfx ($CB:5F)