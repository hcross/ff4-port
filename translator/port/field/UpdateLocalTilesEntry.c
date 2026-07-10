#include "snes/snes.h"

void GetTileProps_c(Snes *snes);  /* $00:9FC2 sibling, ff4-gnw/field/GetTileProps.c */

/* $00:9F6E — refresh the five local tile-property slots around the player.
 * Reads the base tile coordinates from $1706 (x) / $1707 (y), then calls the
 * tile-properties routine ($00:9FC2, GetTileProps_c) for the tile above,
 * left, current, right and below, storing each 16-bit property word to
 * $A3/$A9/$A1/$A5/$A7 and each tile id to $070C/$070F/$070B/$070D/$070E.
 * Five JSR sites in bank $00 ($9BEA/$9D4F/$A94C/$AB8B/$E2E2), on the
 * player-movement paths.
 *
 * ROM-bytes-are-truth: the reference disassembly annotates this routine at
 * $00:9F6C, two bytes early — the bytes there (AD 06 17 read from $9F6C
 * would swallow the real entry) and all five call sites are JSR $9F6E
 * (20 6E 9F). FIFTH off-by-2 of this class. This body REPLACES the retired
 * D1E9F6C dispatch entry, which carried a rewritten bank on top of the
 * off-by-2 ($1E:9F6C is graphics/mask DATA, has zero call sites in the
 * whole ROM, and its "hardcore PASS" spike delegated the inner calls to
 * the equally-wrong $00:9FC0 — vacuous evidence).
 *
 * DP is the caller's D (=$0600 in the field engine); $1A/$1B/$06/$1E and
 * the $A1..$AA result words are direct-page via (dp + offset). $1706/$1707
 * and $070B..$070F are absolute, DB=$00 (WRAM mirror). The inner calls go
 * to the native GetTileProps_c (spike-proven L2), not a delegate. */
void UpdateLocalTiles_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    const uint16_t dp = snes->cpu->dp;
    const uint16_t a1A = (uint16_t)(dp + 0x1A);
    const uint16_t a1B = (uint16_t)(dp + 0x1B);

    /* tile above: (x, y-1) */
    ram[a1A] = ram[0x1706];                                 // LDA $1706; STA $1A
    ram[a1B] = (uint8_t)(ram[0x1707] - 1);                  // LDA $1707; DEC A; STA $1B
    GetTileProps_c(snes);                                   // JSR $9FC2
    ram[(uint16_t)(dp + 0xA3)] = ram[(uint16_t)(dp + 0x1E)];// LDX $1E; STX $A3 (16-bit)
    ram[(uint16_t)(dp + 0xA4)] = ram[(uint16_t)(dp + 0x1F)];
    ram[0x070C] = ram[(uint16_t)(dp + 0x06)];               // LDA $06; STA $070C

    /* tile left: (x-1, y) */
    ram[a1B]++;                                             // INC $1B
    ram[a1A]--;                                             // DEC $1A
    GetTileProps_c(snes);
    ram[(uint16_t)(dp + 0xA9)] = ram[(uint16_t)(dp + 0x1E)];// STX $A9
    ram[(uint16_t)(dp + 0xAA)] = ram[(uint16_t)(dp + 0x1F)];
    ram[0x070F] = ram[(uint16_t)(dp + 0x06)];

    /* current tile: (x, y) */
    ram[a1A]++;                                             // INC $1A
    GetTileProps_c(snes);
    ram[(uint16_t)(dp + 0xA1)] = ram[(uint16_t)(dp + 0x1E)];// STX $A1
    ram[(uint16_t)(dp + 0xA2)] = ram[(uint16_t)(dp + 0x1F)];
    ram[0x070B] = ram[(uint16_t)(dp + 0x06)];

    /* tile right: (x+1, y) */
    ram[a1A]++;                                             // INC $1A
    GetTileProps_c(snes);
    ram[(uint16_t)(dp + 0xA5)] = ram[(uint16_t)(dp + 0x1E)];// STX $A5
    ram[(uint16_t)(dp + 0xA6)] = ram[(uint16_t)(dp + 0x1F)];
    ram[0x070D] = ram[(uint16_t)(dp + 0x06)];

    /* tile below: (x, y+1) */
    ram[a1B]++;                                             // INC $1B
    ram[a1A]--;                                             // DEC $1A
    GetTileProps_c(snes);
    ram[(uint16_t)(dp + 0xA7)] = ram[(uint16_t)(dp + 0x1E)];// STX $A7
    ram[(uint16_t)(dp + 0xA8)] = ram[(uint16_t)(dp + 0x1F)];
    ram[0x070E] = ram[(uint16_t)(dp + 0x06)];
}                                                           // RTS ($00:9FC1)

/* Spike-only shim: the auto-generated spike calls <file-stem>_c. The real
 * function keeps its production name (see the REVERSED_FUNCTION note on why
 * the file stem must differ from the asm label). */
void UpdateLocalTilesEntry_c(Snes *snes) { UpdateLocalTiles_c(snes); }

// PITFALLS: entry is $00:9F6E not $00:9F6C (disassembly off-by-2, ROM bytes
//   are truth — fifth instance); the retired D1E9F6C entry had a rewritten
//   bank on top ($1E:9F6C is data, never a call target); DP is the caller's
//   D (=$0600 field), all $nn via (dp+nn); m=1/x=0 at entry (8-bit A,
//   16-bit X — STX $nn writes two bytes); DEC A and INC/DEC $nn wrap 8-bit.
// HELPERS: GetTileProps_c (native sibling, $00:9FC2, L2)
// SPIKE_EXTRA_SRC: ../../ff4-gnw/field/GetTileProps.c
// SPIKE_COMPARE: region
// CONTRACT:
//   (addresses resolved at the field entry DP=$0600; the C body itself stays
//    dp-relative via cpu->dp. Same tilemap/props fuzz surface as the
//    GetTileProps contract, plus the base coordinates.)
//   inputs_ram:  0x1706=1, 0x1707=1, 0x1700=1, 0x0FDF=1, 0x0EDB=512, 0x15C71=16384
//   output_ram:  0x06A1=10, 0x070B=5, 0x0606=1, 0x0618=2, 0x061A=2, 0x063D=2
//   entry_mode:  mf=true, xf=false, dp=0x0600, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::UpdateLocalTilesEntry ($00:9F6E)
//   (this port file is deliberately named UpdateLocalTilesEntry.c, NOT the
//    asm label UpdateLocalTiles: generate_spike.py derives the spike's target
//    address from the FILE name via ca65-bridge and trusts that over this
//    line, and the bridge resolves `UpdateLocalTiles` to the annotated
//    $00:9F6C — the off-by-2 WRONG address. The production file in
//    ff4-gnw/field/ keeps the plain name.)
