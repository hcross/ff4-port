#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$7E, DP=0
// Conditional branch on A (beq), so Z flag reflects A on entry.
// All inputs/outputs in WRAM.
//   in : ram[$4A] (low 3 bits used)
//   out: ram[$4B,$4D,$4F,$51] updated if condition met
static void UpdateSun_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    uint8_t a = ram[0x4A] & 0x07;  // lda $4a / and #$07
    if (a == 0) return;            // beq @e195

    // Copy 16-bit values from $70/$72 to $4B/$4D
    write16(ram, 0x4B, read16(ram, 0x70));  // ldx $70 / stx $4b
    write16(ram, 0x4D, read16(ram, 0x72));  // ldx $72 / stx $4d

    ram[0x4F] = 0x30;  // lda #$30 / sta $4f
    ram[0x51] = 0x18;  // lda #$18 / sta $51

    DrawSolarSystemSprite_emu(snes);  // jsr DrawSolarSystemSprite
}

// PITFALLS: 1 (DB=$7E required for WRAM access)
// HELPERS: DrawSolarSystemSprite_emu(snes)
// CONTRACT:
//   inputs_reg:  a=8, x=none, y=none
//   inputs_ram:  0x4A=1, 0x70=2, 0x72=2
//   output_ram:  0x4B=2, 0x4D=2, 0x4F=1, 0x51=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: cutscene::UpdateSun ($E1:7C)