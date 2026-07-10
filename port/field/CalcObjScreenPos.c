#include "snes/snes.h"

/* $00:BDB2 — compute the on-screen position of the object indexed by $AF from
 * its map coordinates ($0904,X / $0906,X) and the field scroll/camera
 * ($0C/$5A horizontal, $0E/$5C vertical), writing screen X to $18, screen Y
 * to $1A, and the off-screen flag to $D7 (1 if the object falls outside the
 * 256x240 visible window). Called ~28x/frame on the field map (one per active
 * object); the hottest interpreted routine on first-free-roam.
 *
 * ROM-bytes-are-truth: the reference disassembly decodes this routine two
 * bytes early (it annotates the entry $00:BDB0). The real, sole entry is
 * $00:BDB2 -- it is the only runtime dispatch target, and the bytes at
 * $00:BDB0/BDB1 are 90 60 (the BCC/RTS tail of the preceding routine), not
 * the PHX/PHY the disassembly shows there. Same off-by-2 class as the
 * D00F533/D00F535 finding.
 *
 * DP is the caller's D (=$0600 in the field engine); every direct-page access
 * goes through (dp + offset). The object table is absolute, DB=$00 (WRAM). */
void CalcObjScreenPos_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    const uint16_t dp = snes->cpu->dp;

    ram[(uint16_t)(dp + 0xD7)] = 0;                                 // STZ $D7

    const uint16_t x = (uint16_t)(ram[(uint16_t)(dp + 0xAF)]
                        | (ram[(uint16_t)(dp + 0xB0)] << 8));       // LDX $AF (16-bit)

    // screen X = ((objX & 0xFF) << 4) + scrollX($0C) - cameraX($5A), & 0x03FF
    uint16_t a = (uint16_t)(ram[0x0904 + x] & 0xFF);               // LDA $0904,X; AND #$00FF
    a = (uint16_t)(a << 4);
    a = (uint16_t)(a + (ram[(uint16_t)(dp + 0x0C)] | (ram[(uint16_t)(dp + 0x0D)] << 8)));
    a = (uint16_t)(a - (ram[(uint16_t)(dp + 0x5A)] | (ram[(uint16_t)(dp + 0x5B)] << 8)));
    a &= 0x03FF;
    ram[(uint16_t)(dp + 0x18)] = (uint8_t)a;                        // STA $18 (16-bit)
    ram[(uint16_t)(dp + 0x19)] = (uint8_t)(a >> 8);

    if (a >= 0x0100) {                                             // BCS $BDEE: off-screen (X)
        ram[(uint16_t)(dp + 0xD7)]++;                              // INC $D7
        return;                                                    // (Y is left untouched)
    }

    // screen Y = ((objY & 0xFF) << 4) + scrollY($0E) - cameraY($5C), & 0x03FF
    uint16_t b = (uint16_t)(ram[0x0906 + x] & 0xFF);               // LDA $0906,X; AND #$00FF
    b = (uint16_t)(b << 4);
    b = (uint16_t)(b + (ram[(uint16_t)(dp + 0x0E)] | (ram[(uint16_t)(dp + 0x0F)] << 8)));
    b = (uint16_t)(b - (ram[(uint16_t)(dp + 0x5C)] | (ram[(uint16_t)(dp + 0x5D)] << 8)));
    b &= 0x03FF;
    ram[(uint16_t)(dp + 0x1A)] = (uint8_t)b;                        // STA $1A (16-bit)
    ram[(uint16_t)(dp + 0x1B)] = (uint8_t)(b >> 8);
    if (b >= 0x00F0) ram[(uint16_t)(dp + 0xD7)]++;                  // BCC skips; else INC $D7
}

// PITFALLS: entry is $00:BDB2 not $00:BDB0 (disassembly off-by-2, ROM bytes
//   are truth); DP is the caller's D (=$0600 field), all $nn via (dp+nn);
//   16-bit A across the math (REP #$20), object index X kept to a byte.
// HELPERS: none
// SPIKE_COMPARE: region
// CONTRACT:
//   (addresses resolved at the field entry DP=$0600; the C body itself stays
//    dp-relative via cpu->dp. $06AF fuzzed as a byte so the object index X
//    stays in [0,255] -- a word fuzz would push $0904+X across the bank into
//    ROM and diverge vacuously.)
//   inputs_ram:  0x06AF=1, 0x0904=2, 0x0906=2, 0x060C=2, 0x065A=2, 0x060E=2, 0x065C=2
//   output_ram:  0x0618=2, 0x061A=2, 0x06D7=1
//   entry_mode:  mf=true, xf=false, dp=0x0600, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::CalcObjScreenPos ($00:BDB2)
