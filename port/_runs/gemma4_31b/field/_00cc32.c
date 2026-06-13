#include "snes/snes.h"

// Entry mode: A 8-bit (mf=1), X 16-bit (xf=0), DB=$00, DP=0
// Logic:
//   Initializes several scratchpad areas in WRAM ($0ACF, $0AD2, $0ACD, $0ACE)
//   Calls two sub-routines (_00e075 and _00cd72) to process data.
//   Sets a series of constants into memory locations $EF, $F1, $F3, $F5.
static void _00cc32_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    ram[0x0ACF] = 0x10;                    // lda #$10 / sta $0acf
    write16(ram, 0x0AD2, 0x0010);           // ldx #$0010 / stx $0ad2
    ram[0x0ACD] = 0;                       // stz $0acd
    ram[0x0ACE] = 0;                       // stz $0ace

    _00e075_emu(snes);                      // jsr _00e075
    _00cd72_emu(snes);                      // jsr _00cd72

    write16(ram, 0xEF, 0x0030);             // ldx #$0030 / stx $ef
    write16(ram, 0xF1, 0x00B0);             // ldx #$00b0 / stx $f1
    write16(ram, 0xF3, 0x00B0);             // ldx #$00b0 / stx $f3
    write16(ram, 0xF5, 0xFFE0);             // ldx #$ffe0 / stx $f5
}

// PITFALLS: None. Standard linear initialization.
// HELPERS: _00e075_emu(snes), _00cd72_emu(snes)
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x0ACF=1, 0x0AD2=2, 0x0ACD=1, 0x0ACE=1, 0xEF=2, 0xF1=2, 0xF3=2, 0xF5=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x0
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::_00cc32 ($CC:3D)