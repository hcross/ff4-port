#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$CB, DP=0
// This routine initializes a series of memory locations at DP $0A00
// with constants before calling two sub-routines.
static void _00cb72_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // Call setup routine _00cd72
    _00cd72_emu(snes);

    ram[0x0ACF] = 0x08;               // lda #$08 / sta $0acf
    write16(ram, 0x0AD2, 0x0010);     // ldx #$0010 / stx $0ad2
    ram[0x0ACD] = 0;                  // stz $0acd
    ram[0x0ACE] = 0;                  // stz $0ace
    ram[0x0AD0] = 0x02;               // lda #$02 / sta $0ad0
    ram[0x0AD1] = 0x02;               // sta $0ad1
    ram[0x0AD4] = 0x60;               // lda #$60 / sta $0ad4
    ram[0x0AD5] = 0x60;               // sta $0ad5

    // Call processing routine _00e075
    _00e075_emu(snes);
}

// PITFALLS: 1 (DB=$CB required for DP offsets $0Axx), 6 (Mode A 8-bit assumed)
// HELPERS: _00cd72_emu(snes), _00e075_emu(snes), write16()
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x0ACF=1, 0x0AD2=2, 0x0ACD=1, 0x0ACE=1, 0x0AD0=1, 0x0AD1=1, 0x0AD4=1, 0x0AD5=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0xCB
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00cb72 ($CB:72)