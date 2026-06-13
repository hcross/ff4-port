#include "snes/snes.h"

// Sets $91 = 0x39, then tail-calls _d9e5 with A low byte = 0xC0.
// In 8-bit A mode, the high byte of A is preserved from entry and
// passed through to _d9e5.
static void Special_06_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *c = snes->cpu;
    ram[0x91] = 0x39;
    c->a = (c->a & 0xFF00) | 0xC0;  // lda #$c0: 8-bit, high byte preserved (Pitfall 9)
    c->z = false;                   // 0xC0 != 0
    c->n = true;                    // 0xC0 & 0x80 != 0
    _d9e5_emu(snes);                // jmp _d9e5 (tail call)
}

// PITFALLS: 9 (high byte of A preserved across 8-bit lda #$c0, passed to _d9e5)
// HELPERS: _d9e5_emu(snes) — delegates _d9e5 (jump target)
// CONTRACT:
//   inputs_reg:  a=16 (high byte preserved and forwarded), x=none, y=none
//   inputs_ram:  none
//   output_ram:  0x91=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::Special_06 ($D9:D6)