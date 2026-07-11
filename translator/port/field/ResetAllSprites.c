#include "snes/snes.h"

/* $00:9179 — reset all sprites: park every entry of the OAM shadow table
 * off-screen (y = $F0 at $0301 + 4k for the 128 sprites) and clear the
 * 32-byte high-OAM shadow at $0500. Called once per frame by the field
 * engine's sprite rebuild (37 JSR sites in bank $00) -- measured at ~25%
 * of ALL interpreted opcodes on 009-first-free-roam post-M2 (PC
 * histogram, 2026-07-11): the two tight store loops are pure interpreter
 * overhead for what is a fill.
 *
 * ROM-bytes-are-truth: the reference disassembly annotates the entry at
 * $00:9177; every one of the 37 call sites is JSR $9179 (20 79 91) and
 * $9177 has zero callers. Sixth off-by-2 of the D00F533/F535 class.
 *
 * All stores are absolute ($0301,X / $0500,X with DB=$00 -> low-WRAM
 * mirror; DB=$7E would hit the same bytes) -- no direct-page access, so
 * the body is DP-insensitive. Callers never consume the exit A/X (spot-
 * checked continuations: immediate reloads or new calls). */
void ResetAllSprites_c(Snes *snes) {
    uint8_t *ram = snes->ram;
    for (int x = 0; x < 0x200; x += 4)                  // LDA #$F0; STA $0301,X; 4x INX; CPX #$0200; BNE
        ram[0x0301 + x] = 0xF0;
    memset(&ram[0x0500], 0, 0x20);                      // STZ $0500,X; INX; CPX #$0020; BNE
}

// PITFALLS: entry is $00:9179 not $00:9177 (disassembly off-by-2, ROM bytes
//   are truth -- sixth instance); m=1/x=0 at entry (8-bit stores, 16-bit
//   CPX); all stores absolute, no DP dependence.
// HELPERS: none
// SPIKE_COMPARE: region
// CONTRACT:
//   (both target regions are fuzzed as inputs so the fill is exercised
//    over varied prior state, not a zero page.)
//   inputs_ram:  0x0301=512, 0x0500=32
//   output_ram:  0x0301=512, 0x0500=32
//   entry_mode:  mf=true, xf=false, dp=0x0600, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::ResetAllSprites ($00:9179)
