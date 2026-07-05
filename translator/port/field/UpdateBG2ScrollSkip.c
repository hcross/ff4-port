#include "snes/snes.h"

/* ============================ FIX APPLIED (2026-07-05) ============================
 * Was previously guard-from-A ("skip the lda $c9" model) — WRONG. Confirmed
 * by direct ROM-byte inspection: ff4-jp1.sfc (headerless LoROM) has the
 * pattern `A5 C9 D0 0E AD 00 17 C9 03` (lda $c9 / bne / lda $1700 / cmp #3)
 * at file offset 0x7535 => SNES $00:F535. The real $00:F535 instruction is
 * `LDA $C9` -- it always loads its own guard byte, exactly like
 * UpdateBG2Scroll_c. The disassembly's two-entry model ($00:F533 = "full"
 * entry loading $C9, $00:F535 = "skip" entry reading A) was off by 2 bytes:
 * $00:F533 is not even an instruction boundary (operand byte of the
 * PRECEDING routine's LDX $43 at $00:F532). The game's only callers,
 * `JSR $F535` at $00:84FE and $00:9391, both hit the real lda-$C9 entry.
 * Dispatch fixed to match: dispatch_all.c's dead { 0x00f533, ... } entry
 * removed; { 0x00f535, UpdateBG2ScrollSkip_c } kept (name unchanged --
 * registry_promote.py can't rename a dispatch ID's `name` field -- but the
 * body below now reads $C9 like it always should have.
 * Proof: guard-from-A vs the real $00:F535 asm failed at scale (17/5000,
 * this session's own re-runs: 2/500, 4/1000); guard-from-$C9 vs the SAME
 * asm = 0/2000 (independently re-verified before applying this fix).
 * Full writeup: MemPalace wing=ff4-gnw room=obstacles-and-solutions,
 * "UpdateBG2ScrollSkip_c REAL DISPATCH BUG".
 * ==================================================================== */

/*
 * Helpers for 16-bit DP/WRAM access.
 * All DP-relative reads/writes pass (dp + offset) so the caller's D register
 * is honoured (the field engine runs with D=$0600, not D=$0000).
 * Absolute addresses ($1700, $0FE4) bypass dp.
 */
static inline uint16_t r16(const uint8_t *ram, uint16_t addr) {
    return (uint16_t)(ram[addr] | ((uint16_t)ram[(uint16_t)(addr + 1)] << 8));
}
static inline void w16(uint8_t *ram, uint16_t addr, uint16_t v) {
    ram[addr]     = (uint8_t)(v & 0xFF);
    ram[addr + 1] = (uint8_t)(v >> 8);
}

/*
 * ROM-constant BG2 scroll tables (bank $00):
 *   $F5F9: speed mask per bits[7:6]      —  db $00, $07, $01, $00
 *   $F5FD: h-scroll delta per bits[5:4]  —  dw 0, -1, 0, +1  (16-bit LE)
 *   $F605: v-scroll delta per bits[5:4]  —  dw +1, 0, -1, 0  (16-bit LE)
 *
 * The 65816 computes byte offset Y = (fe4 & 0x30) >> 3 for 16-bit tables.
 * In C we fold to index (fe4 & 0x30) >> 4.
 */
static const uint8_t k_speed_mask[4] = { 0x00, 0x07, 0x01, 0x00 };
static const int16_t k_h_delta[4]    = { 0, -1, 0, 1 };
static const int16_t k_v_delta[4]    = { 1, 0, -1, 0 };

/* Shared body entered after the guard BNE ($F535 onward).
 *
 * guard_val: the value tested by BNE — 0 = proceed, non-zero = early RTS.
 *
 * IMPORTANT: CPU mode flags and DP are NOT overridden here.  The caller
 * runs with D=$0600 (field engine context); all DP-relative accesses must
 * go through dp, not hardcoded $00xx offsets.  We only override mf/xf/db
 * when (and only when) the body actually does 16-bit arithmetic — i.e. after
 * all guard checks pass — and we restore them (via SEP/REP) only when the
 * actual code path does so (continuous scroll path ends with SEP #$20). */
static void update_bg2_scroll_body(Snes *snes, uint8_t guard_val) {
    if (guard_val) return;   /* BNE taken: caller dp/mf unchanged */

    uint8_t  *ram = snes->ram;
    uint16_t  dp  = snes->cpu->dp;   /* D register — $0600 in field context */

    /* Absolute addresses (no dp offset): $1700 and $0FE4 */
    if (ram[0x1700] != 0x03) return;

    uint8_t fe4 = ram[0x0FE4];
    if (!(fe4 & 0xC0)) return;   /* AND #$C0 / BNE proceed */

    /* All paths below modify WRAM and do 16-bit arithmetic.
     * Set the mode flags the routine sets with REP #$20 / SEP #$20. */
    snes->cpu->mf = false;   /* REP #$20 in effect for 16-bit ADC/STA/LDA */
    snes->cpu->xf = false;   /* X/Y remain 16-bit throughout */
    snes->cpu->db = 0;

    if (fe4 & 0x06) {
        /* Parallax path ($F593+) — does 16-bit LDX/STX, 8-bit shifts ------ */
        snes->cpu->mf = true;   /* no REP #$20 in the parallax path */
        uint8_t scale = fe4 & 0xC0;

        if (fe4 & 0x04) {
            /* h-parallax: $5E = $5A, scale by bits[7:6] */
            w16(ram, (uint16_t)(dp + 0x5E), r16(ram, (uint16_t)(dp + 0x5A)));
            if (scale != 0x80) {
                uint16_t v = r16(ram, (uint16_t)(dp + 0x5E));
                w16(ram, (uint16_t)(dp + 0x5E),
                    (scale == 0x40) ? (uint16_t)(v >> 1) : (uint16_t)(v << 1));
            }
        }

        if ((fe4 & 0x06) == 0x04) {
            w16(ram, (uint16_t)(dp + 0x60), 0);
            return;
        }

        if (fe4 & 0x02) {
            w16(ram, (uint16_t)(dp + 0x60), r16(ram, (uint16_t)(dp + 0x5C)));
            if (scale != 0x80) {
                uint16_t v = r16(ram, (uint16_t)(dp + 0x60));
                w16(ram, (uint16_t)(dp + 0x60),
                    (scale == 0x40) ? (uint16_t)(v >> 1) : (uint16_t)(v << 1));
            }
            if ((fe4 & 0x06) == 0x02) {
                w16(ram, (uint16_t)(dp + 0x5E), 0);
            }
        }
        return;
    }

    /* Continuous scroll path ($F54D+) — REP #$20 in effect for 16-bit ADC */
    uint8_t speed_idx = (fe4 & 0xC0) >> 6;
    if (!(ram[(uint16_t)(dp + 0x7A)] & k_speed_mask[speed_idx])) {
        uint8_t dir_idx = (fe4 & 0x30) >> 4;
        w16(ram, (uint16_t)(dp + 0x66),
            (uint16_t)(r16(ram, (uint16_t)(dp + 0x66)) + (uint16_t)k_h_delta[dir_idx]));
        w16(ram, (uint16_t)(dp + 0x68),
            (uint16_t)(r16(ram, (uint16_t)(dp + 0x68)) + (uint16_t)k_v_delta[dir_idx]));
    }
    w16(ram, (uint16_t)(dp + 0x5E),
        (uint16_t)(r16(ram, (uint16_t)(dp + 0x5A)) + r16(ram, (uint16_t)(dp + 0x66))));
    w16(ram, (uint16_t)(dp + 0x60),
        (uint16_t)(r16(ram, (uint16_t)(dp + 0x5C)) + r16(ram, (uint16_t)(dp + 0x68))));

    /* Matches $F58B: LDA #0 / SEP #$20 before RTS in the continuous path */
    snes->cpu->mf = true;
    snes->cpu->a  = (snes->cpu->a & 0xFF00); /* low byte = 0 */
}

/* Bank $16 entry ($16:F533, dispatch D16F533) — untouched, separate, valid. */
void UpdateBG2Scroll_c(Snes *snes) {
    update_bg2_scroll_body(snes, snes->ram[(uint16_t)(snes->cpu->dp + 0xC9)]);
}

/* Bank $00's sole real entry point, dispatched at $00:F535 (D00F535).
 * Name kept as UpdateBG2ScrollSkip_c (registry name field not renameable
 * via registry_promote.py) but the body now reads $C9 itself -- see the
 * FIX APPLIED note above. */
void UpdateBG2ScrollSkip_c(Snes *snes) {
    update_bg2_scroll_body(snes, snes->ram[(uint16_t)(snes->cpu->dp + 0xC9)]);
}

// SPIKE_COMPARE: region
// CONTRACT:
//   inputs_ram:  0x00C9=1, 0x1700=1, 0x0FE4=1, 0x005A=2, 0x005C=2, 0x0066=2, 0x0068=2, 0x007A=1
//   output_ram:  0x005E=2
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x00
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: field::UpdateBG2Scroll ($00:F535)
//
// This is the $F535 entry point, now spiked with its real behavior (loads
// its own $C9 guard, exactly like the $16:F533 entry). dp is forced to 0 so
// the dp-relative body accesses land on the numeric addresses declared
// above (the routine is DP_SENSITIVE -- D is caller-supplied $0600 in the
// field engine, but the arithmetic itself is dp-agnostic).
