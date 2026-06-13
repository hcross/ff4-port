#include "snes/snes.h"

// MagicEffect_24: compute 16-bit difference ($2709 - $2707), store at $A4,
// then set bit 7 of the high byte ($A5) and tail-call SetMagicStatus.
static void MagicEffect_24_c(Snes *snes) {
    uint8_t *ram = snes->ram;

    // 16-bit subtraction (sec then sbc with C=1 → no borrow)
    uint16_t val2709 = read16(ram, 0x2709);
    uint16_t val2707 = read16(ram, 0x2707);
    uint16_t diff = val2709 - val2707;
    write16(ram, 0xA4, diff);

    // ora #$80 on the high byte (A5)
    ram[0xA5] |= 0x80;

    // Tail call: SetMagicStatus expects A = (ram[$A5] | 0x80) in 8-bit mode,
    // B = 0 (from shorta0), flags Z/N reflecting that value.
    Cpu *c = snes->cpu;
    c->dp = 0;
    c->db = 0x7E;
    c->mf = true;   // shorta0 left A 8-bit
    c->xf = false;  // battle convention: X/Y 16-bit
    c->a = (uint16_t)ram[0xA5];   // lda $a5 after ora, B=0
    c->z = (c->a == 0);
    c->n = (c->a & 0x80) != 0;
    set_magic_status_emu(snes);   // jmp SetMagicStatus
}

// PITFALLS: none (no conditional branch at entry, mode overrides are explicit,
//           subtraction is direct 16-bit wrap, no hidden B issues)
// HELPERS: read16/write16 (inline), set_magic_status_emu(snes) — delegates SetMagicStatus @ $DD:05
// CONTRACT:
//   inputs_reg:  a=none, x=none, y=none
//   inputs_ram:  0x2707=2, 0x2709=2
//   output_ram:  0xA4=2   (diff with high byte bit7 set; SetMagicStatus delegated)
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E   (battle defaults; routine overrides A mode internally)
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: battle::MagicEffect_24 ($DD:06)