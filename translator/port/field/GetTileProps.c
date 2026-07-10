#include "snes/snes.h"

/* $00:9FC2 — get the properties of the map tile at ($1A, $1B). Looks the tile
 * id up in the bg tilemap at $7F:5C71 + (y<<8 | x), stores it to $06, then
 * fetches the 16-bit property word from the $0EDB table (indexed by tile*2)
 * into $1E/$1F. On the world map ($1700 != 3) y is wrapped mod 64; on a
 * sub-map, coordinates outside [0,0x20) short-circuit to a fixed property
 * word ($0007, or $0000 when $0FDF bit 7 marks out-of-bounds tiles solid)
 * without touching $06/$18/$3D/$3E. Called 5x per movement step (~10x/frame
 * on first-free-roam) by the world/sub-map tile-passability code.
 *
 * ROM-bytes-are-truth: the reference disassembly annotates this routine at
 * $00:9FC0, two bytes early — the bytes there (07 60) are the STA $070E / RTS
 * tail of the preceding routine, and all five call sites in bank $00 are
 * JSR $9FC2 (20 C2 9F). FOURTH off-by-2 of this class (D00F533/F535,
 * D00BDB2, D00C357). The annotated "JMP $9FF1" is really JMP $9FF3, an
 * internal jump into the routine's own common tail — no external caller
 * targets it.
 *
 * DP is the caller's D (=$0600 in the field engine); every direct-page access
 * goes through (dp + offset). $1700/$0FDF/$0EDB,X are absolute, DB=$00 (WRAM
 * mirror). The callers reload A and X from $06/$1E right after the JSR, so
 * exit register values are not live. */
void GetTileProps_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    const uint16_t dp = snes->cpu->dp;

    uint8_t ylo;
    if (ram[0x1700] != 0x03) {                                      // LDA $1700; CMP #$03; BEQ $9FD0
        ylo = (uint8_t)(ram[(uint16_t)(dp + 0x1B)] & 0x3F);         // LDA $1B; AND #$3F (y mod 64)
                                                                    // JMP $9FF3 (common tail)
    } else {
        const uint8_t x = ram[(uint16_t)(dp + 0x1A)];               // LDA $1A
        const uint8_t y = ram[(uint16_t)(dp + 0x1B)];               // LDA $1B
        if ((x & 0x80) || x >= 0x20 || (y & 0x80) || y >= 0x20) {   // BMI/BCS/BMI/BCC bounds gate
            const uint16_t props =
                (ram[0x0FDF] & 0x80) ? 0x0000 : 0x0007;             // LDA $0FDF; BPL $9FEB
            ram[(uint16_t)(dp + 0x1E)] = (uint8_t)props;            // LDX #imm; STX $1E (16-bit)
            ram[(uint16_t)(dp + 0x1F)] = (uint8_t)(props >> 8);
            return;                                                 // RTS
        }
        ylo = y;                                                    // $9FF1: LDA $1B
    }

    ram[(uint16_t)(dp + 0x3E)] = ylo;                               // STA $3E
    ram[(uint16_t)(dp + 0x3D)] = ram[(uint16_t)(dp + 0x1A)];        // LDA $1A; STA $3D
    const uint16_t idx = (uint16_t)(ram[(uint16_t)(dp + 0x3D)]
                          | (ram[(uint16_t)(dp + 0x3E)] << 8));     // LDX $3D (16-bit)

    const uint8_t tile = ram[0x10000 + 0x5C71 + idx];               // LDA $7F5C71,X (bg tilemap)
    ram[(uint16_t)(dp + 0x06)] = tile;                              // STA $06
    const uint16_t t2 = (uint16_t)(tile << 1);                      // STA $18; STZ $19; ASL $18; ROL $19
    ram[(uint16_t)(dp + 0x18)] = (uint8_t)t2;
    ram[(uint16_t)(dp + 0x19)] = (uint8_t)(t2 >> 8);
    ram[(uint16_t)(dp + 0x1E)] = ram[0x0EDB + t2];                  // LDX $18; LDA $0EDB,X; STA $1E
    ram[(uint16_t)(dp + 0x1F)] = ram[0x0EDC + t2];                  // LDA $0EDC,X; STA $1F
}

// PITFALLS: entry is $00:9FC2 not $00:9FC0 (disassembly off-by-2, ROM bytes
//   are truth — fourth instance of this class); DP is the caller's D (=$0600
//   field), all $nn via (dp+nn); m=1/x=0 at entry (8-bit A, 16-bit X);
//   ASL $18/ROL $19 is a 16-bit shift across the byte pair.
// HELPERS: none
// SPIKE_COMPARE: region
// CONTRACT:
//   (addresses resolved at the field entry DP=$0600; the C body itself stays
//    dp-relative via cpu->dp. The tilemap slice covers the full reachable
//    index range — world map caps y at 0x3F so idx <= 0x3FFF; the props
//    table is 256 tiles x 2 bytes.)
//   inputs_ram:  0x1700=1, 0x0FDF=1, 0x061A=1, 0x061B=1, 0x0EDB=512, 0x15C71=16384
//   output_ram:  0x0606=1, 0x0618=2, 0x061E=2, 0x063D=1, 0x063E=1
//   entry_mode:  mf=true, xf=false, dp=0x0600, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::GetTileProps ($00:9FC2)
