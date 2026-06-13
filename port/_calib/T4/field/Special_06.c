#include "snes/snes.h"

static void Special_06_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    Cpu *c = snes->cpu;
    ram[0x91] = 0x39;
    c->a = 0xC0;          // lda #$c0
    c->n = true;          // $c0 has bit 7 set
    c->z = false;         // not zero
    // DB? Not needed for direct page, but for consistency:
    // c->db = ???; // unknown for field, but _d9e5_emu will set its own if needed?
    _d9e5_emu(snes);
}