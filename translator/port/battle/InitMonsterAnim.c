#include "snes/snes.h"
#include "snes/cart.h"

/* Standalone spike extraction of InitMonsterAnim_c (= asm @ $02:DAFE),
 * bundled in ff4-gnw/battle/btlgfx_monsters.c. The body is copied verbatim
 * except for the 8-bit index-truncation fix on `mon47 << 2`: the asm computes
 * it in an 8-bit accumulator (LDA $47 / ASL / ASL / TAX, mf=true), so it wraps
 * at 256. The un-truncated uint16_t shift diverges for $47 >= 0x40 — exactly
 * the BuildOAMEntries_c width bug class. inject_cycles is stubbed no-op: the
 * region spike compares post-state, not cycle timing (this routine only writes
 * the WRAM cells $EFCE/$EFCF/$EFD1/$EFD3 and fires no HDMA/NMI). */

#define LOROM(bank, addr) (((uint32_t)(bank) << 15) | ((addr) & 0x7FFF))
static inline void inject_cycles(Snes *snes, int n) { (void)snes; (void)n; }

void InitMonsterAnim_c(Snes *snes) {
    uint8_t mon_x = (uint8_t)(snes->cpu->x);  /* entry X = slot index */
    uint8_t mon47 = snes->ram[0x47];

    /* Prologue guard: check $F015[mon47*4] bits[7:6] and $F2BC[mon47] */
    uint16_t xi4 = (uint16_t)(uint8_t)(mon47 << 2);  /* asm: LDA $47/ASL/ASL/TAX = 8-bit, wraps at 256 */
    if (snes->ram[0xF015 + xi4] & 0xC0) {
        /* Early exit A: bits set — PHX(30)+LDA_dp(24)+ASL(14)+ASL(14)+TAX(14)+
         * LDA_abs_X(38)+AND_imm(16)+BNE_t(22)+PLX(36)+RTS(42) = 250; inject=234 */
        inject_cycles(snes, 234);
        return;
    }
    if (snes->ram[0xF2BC + mon47]) {
        /* Early exit B: F2BC non-zero — add: BEQ_nt(16)+LDA_dp(24)+TAX(14)+LDA_abs_X(38)+BNE_t(22)+PLX(36)+RTS(42)
         * Total = 250+16+24+14+38+22 = 364... recalc full:
         * PHX(30)+LDA_dp(24)+ASL(14)+ASL(14)+TAX(14)+LDA_abs_X(38)+AND_imm(16)+BNE_nt(16)
         * +LDA_dp(24)+TAX(14)+LDA_abs_X(38)+BNE_t(22)+PLX(36)+RTS(42) = 342; inject=326 */
        inject_cycles(snes, 326);
        return;
    }

    /* PLX at $DB15: restore X = mon_x (already in mon_x; no CPU state to update here) */

    uint8_t efc5 = snes->ram[0xEFC5 + mon_x];

    if (efc5 == 0xD0 || efc5 == 0xE0) {
        /* "D0/E0" animation class ($DB24) */
        uint8_t speed = snes->ram[0xF014]
                        ? snes->cart->rom[LOROM(0x16, 0xFD24) + mon47]
                        : snes->cart->rom[LOROM(0x16, 0xFD1F) + mon47];

        if (speed == efc5) {
            /* speed matches current position → treat as "other" class (BEQ $DB60)
             * Fall through to the "other" ($DB60) block below with the current efc5.
             * But first: account for cycles up to this point and then go to DB60 block.
             * Rather than duplicating, we handle this as a goto to the DB60 path.
             * Per disasm: BEQ $DB60 means the "other" block logic runs from there. */
            goto db60_class;
        }

        /* Update EFCE: clear low nibble, then set flag bits */
        uint8_t ctrl = snes->ram[0xEFCE + mon_x];
        snes->ram[0xEFCE + mon_x] = ctrl & 0x0F; /* clear high nibble of low nibble... wait: AND #$0F keeps low nibble */

        if (speed == 0xD0) {
            /* A9 08 → $08, BRA $DBB2 */
            snes->ram[0xEFCF + mon_x] = 0x08;
        } else {
            /* ORA #$20, STA, A=$08, BRA */
            snes->ram[0xEFCE + mon_x] |= 0x20;
            snes->ram[0xEFCF + mon_x] = 0x08;
        }
        goto dbb5_block; /* common F015,Y attribute check */
    }

db60_class:;
    {
        /* "Other" animation class ($DB60): check $F0AF[mon47]
         * $DB60: PHX; LDA $47; TAX; LDA $F0AF,X
         * BEQ $DB6C → PLX; BRA $DBD9 → RTS (efc5 already set up, nothing more to do)
         * BNE $DB6F → continue with "other" initialization path
         */
        uint8_t f0af = snes->ram[0xF0AF + mon47];
        if (f0af) {
            /* $F0AF non-zero: go to EFC5 comparison path ($DB6F) */
            if (efc5 == 0xC0) {
                /* $DB74: CMP #$C0 / BEQ $DBD9 → RTS — done */
                goto dafe_done;
            }
            /* $DB76: PHA; LDA $EFCE,X; AND #$0F; STA $EFCE,X; PLA */
            {
                uint8_t ctrl = snes->ram[0xEFCE + mon_x];
                snes->ram[0xEFCE + mon_x] = ctrl & 0x0F;
            }
            snes->ram[0xEFD1 + mon_x] = 0;
            snes->ram[0xEFD3 + mon_x] = 0;

            if (efc5 == 0xB0) {
                /* $DB86: CMP #$B0 / BEQ $DB8A: ORA #$20 / STA $EFCE,X */
                /* Actually $DB8A: ORA #$20 + STA + A=$10? Let me re-check disasm:
                 * $DB8A: LDA $EFCE,X; ORA #$20; STA $EFCE,X
                 * $DB8F: 9D CE EF = STA $EFCE,X? Already done. Next:
                 * $DB92: A9 10 = LDA #$10; $DB94: 80 19 = BRA $DBAF
                 * $DBAF: SEC; SBC #$08; STA $EFCF,X */
                /* So for B0: set bit5 of EFCE, then A=$10, BRA $DBAF: SEC-SBC #$08 = $10-$08=$08 */
                snes->ram[0xEFCE + mon_x] |= 0x20;
                snes->ram[0xEFCF + mon_x] = (uint8_t)(0x10 - 0x08);  /* = $08 */
                goto dbb5_block;
            }
            /* generic: load speed from table FCEE/FCF3 → SEC-SBC #$08 → EFCF */
            {
                uint8_t raw_speed = snes->ram[0xF014]
                                    ? snes->cart->rom[LOROM(0x16, 0xFCF3) + mon47]
                                    : snes->cart->rom[LOROM(0x16, 0xFCEE) + mon47];
                snes->ram[0xEFCF + mon_x] = (uint8_t)(raw_speed - 8);
            }
            goto dbb5_block;
        } else {
            /* $DB6C: PLX; BRA $DBD9 → $DBD9: RTS. DAFE returns without further writes. */
            goto dafe_done;
        }
    }

dbd9_path:;
    {
        /* Second-object path reached from dbd9_path label — this is DBDA's body,
         * NOT the DAFE $DBD9 RTS. Only called when InitMonsterAnim_c is used as
         * a standalone sub (not as DAFE). For DAFE, F0AF==0 takes goto dafe_done above.
         * This block implements $DBDA..DC48 for completeness.
         * Check F2BC and F0AF for mon47 again */
        if (snes->ram[0xF2BC + mon47] || snes->ram[0xF0AF + mon47]) {
            /* PLX + RTS (early from DBD9) */
            goto dafe_done;
        }
        /* Clear EFCE low nibble */
        {
            uint8_t ctrl = snes->ram[0xEFCE + mon_x];
            snes->ram[0xEFCE + mon_x] = ctrl & 0x0F;
        }

        if (efc5 == 0xB0) {
            /* $DC48: set bit0 of EFCE, RTS */
            snes->ram[0xEFCE + mon_x] |= 0x01;
            goto dafe_done;
        }

        snes->ram[0xEFD1 + mon_x] = 0;
        snes->ram[0xEFD3 + mon_x] = 0;

        uint8_t raw_speed;
        if (efc5 == 0xC0) {
            raw_speed = 0x08;
        } else {
            raw_speed = snes->ram[0xF014]
                        ? snes->cart->rom[LOROM(0x16, 0xFCF3) + mon47]
                        : snes->cart->rom[LOROM(0x16, 0xFCEE) + mon47];
        }
        snes->ram[0xEFCF + mon_x] = raw_speed;
        goto dc23_block; /* F015,Y attribute block (mirrored from dbb5_block) */
    }

dbb5_block:;
    {
        /* $DBB5: LDA $F015,Y / AND #$30 / branch on bits 5-4
         * Y here is caller's Y at entry — passed implicitly as snes->cpu->y */
        uint16_t cal_y = snes->cpu->y;
        uint8_t attr30 = snes->ram[0xF015 + cal_y] & 0x30;

        if (attr30 == 0) {
            /* bits5-4 clear: set bit6 of EFCE */
            snes->ram[0xEFCE + mon_x] |= 0x40;
        } else if (attr30 & 0x20) {
            /* bit5 set: set bit4 of EFCE, then double EFCF */
            snes->ram[0xEFCE + mon_x] |= 0x10;
            snes->ram[0xEFCF + mon_x] = (uint8_t)(snes->ram[0xEFCF + mon_x] << 1);
        } else {
            /* bit4 only: just double EFCF */
            snes->ram[0xEFCF + mon_x] = (uint8_t)(snes->ram[0xEFCF + mon_x] << 1);
        }
        goto dafe_done;
    }

dc23_block:;
    {
        /* $DC23 (mirrored dbb5 logic for the DBD9 sub-path) */
        uint16_t cal_y = snes->cpu->y;
        uint8_t attr30 = snes->ram[0xF015 + cal_y] & 0x30;

        if (attr30 == 0) {
            snes->ram[0xEFCE + mon_x] |= 0x40;
        } else if (attr30 & 0x20) {
            snes->ram[0xEFCE + mon_x] |= 0x10;
            snes->ram[0xEFCF + mon_x] = (uint8_t)(snes->ram[0xEFCF + mon_x] << 1);
        } else {
            snes->ram[0xEFCF + mon_x] = (uint8_t)(snes->ram[0xEFCF + mon_x] << 1);
        }
    }

dafe_done:;
    /* Cycle accounting (M=8, X=16, JSR → inject = body - 16):
     *
     * Early exits:
     *   A: $F015 bits C0 set → 250 MC (inject=234)
     *   B: $F2BC non-zero   → 342 MC (inject=326)
     *
     * Main paths (approximate, dominant path through DB60 class):
     *   Entry to DB15: PHX(30)+LDA_dp(24)+ASL(14)+ASL(14)+TAX(14)+LDA_abs_X(38)
     *                  +AND_imm(16)+BNE_nt(16)+LDA_dp(24)+TAX(14)+LDA_abs_X(38)+BEQ_t(22)+PLX(36) = 300
     *   DB16 → JMP DB60: LDA_abs_X(38)+CMP_imm(16)+BNE_nt(16)+CMP_imm(16)+BNE_nt(16)+JMP_abs(24) = 126
     *   DB60 (F0AF!=0): PHX(30)+LDA_dp(24)+ASL×2(28)+TAX(14)+LDA_abs_X(38)+BEQ_nt(16)+PLX(36)+BRA(22) = 208
     *   DB6F (not C0): LDA_abs_X(38)+CMP_imm(16)+BNE_nt(16) = 70
     *   DB76: PHA(22)+LDA_abs_X(38)+AND_imm(16)+STA_abs_X(38)+PLA(28) = 142
     *   DB80: STZ_abs_X(34)+STZ_abs_X(34)+CMP_imm(16)+BNE_t(22) [not B0] = 106
     *   DB96 (F014=0): LDA_abs(32)+BNE_nt(16)+PHX(30)+LDA_dp(24)+TAX(14)+LDA_long_X(40)+PLX(36)+BRA(22) = 214
     *   DBAF: SEC(14)+SBC_imm(16)+STA_abs_X(38) = 68
     *   DBB5 (bits5-4=0): LDA_abs_Y(38)+AND_imm(16)+BEQ_t(22) = 76 → DBD1: LDA_abs_X(38)+ORA_imm(16)+STA_abs_X(38)+RTS(42) = 134
     *   Total: 300+126+208+70+142+106+214+68+76+134 = 1444 MC → inject=1428
     *
     *   DB6F (EFC5=C0, → DBD9): LDA_abs_X(38)+CMP_imm(16)+BEQ_t(22)+PHX(30)+LDA_dp(24)+ASL×2+TAX+LDA_abs_X(38)+BNE_nt(16)+LDA_abs_X(38)+BEQ_t(22)+PLX(36)+...
     *
     * Strategy: compute dynamically based on actual path taken.
     * Note: all paths computed as full body cycles; inject = cy - 16.
     */
    {
        uint32_t cy;

        /* Classify which path was taken */
        uint16_t xi4_l = (uint16_t)(uint8_t)(mon47 << 2);  /* 8-bit truncation, matches asm */
        bool bits_c0 = (snes->ram[0xF015 + xi4_l] & 0xC0) != 0;
        bool f2bc_nz = (snes->ram[0xF2BC + mon47] != 0);
        /* Note: these were evaluated before we modified RAM, so re-read is safe
         * (we only write EFCE/EFCF/EFD1/EFD3, not F015/F2BC/F0AF) */

        if (bits_c0) {
            cy = 250;
        } else if (f2bc_nz) {
            cy = 342;
        } else {
            /* Entry to DB15 = 300 */
            cy = 300;

            if (efc5 == 0xD0 || efc5 == 0xE0) {
                /* DB24 class: DB16→DB24 = LDA_abs_X(38)+CMP_imm(16)+BEQ_t(22) [or BNE_nt(16)+CMP+BEQ_t] */
                if (efc5 == 0xD0) {
                    cy += 38+16+22;  /* DB16: CMP D0 BEQ taken = 76 */
                } else {
                    cy += 38+16+16+16+22;  /* DB16: CMP D0 BNE_nt, CMP E0 BEQ_t = 108 */
                }
                /* DB24: F014 check + table lookup */
                bool f014 = (snes->ram[0xF014] != 0);
                uint8_t speed_val = f014 ? snes->cart->rom[LOROM(0x16, 0xFD24) + mon47]
                                         : snes->cart->rom[LOROM(0x16, 0xFD1F) + mon47];
                if (!f014) {
                    cy += 32+16+30+24+14+40+36+22;  /* LDA_abs(32)+BNE_nt(16)+PHX(30)+... = 214 (table A path) */
                } else {
                    cy += 32+22+30+24+14+40+36;     /* LDA_abs(32)+BNE_t(22)+PHX(30)+LDA_dp(24)+TAX(14)+LDA_long_X(40)+PLX(36) = 198 (table B path, no BRA) */
                }
                /* CMP $EFC5,X: 38; BEQ (taken if speed==efc5, nt otherwise) */
                if (speed_val == efc5) {
                    cy += 38+22;  /* CMP+BEQ_t(22) → goto DB60 */
                    /* then DB60 path: PHX(30)+LDA_dp(24)+ASL×2(28)+TAX(14)+LDA_abs_X(38)+BEQ_nt/t+PLX(36)+BRA/nop */
                    goto d0e0_to_db60;
                } else {
                    cy += 38+16;  /* CMP+BEQ_nt(16) */
                    /* DB42: PHA(22)+LDA_abs_X(38)+AND_imm(16)+STA_abs_X(38)+PLA(28)+CMP_imm(16)+... */
                    cy += 22+38+16+38+28+16;  /* = 158 */
                    if (speed_val == 0xD0) {
                        cy += 16+22;   /* BNE_nt+LDA_imm+BRA = 16+16+22 = 54... */
                        /* Actually: D0 04=BNE $DB54 (not-taken if ==D0, since we BNE means !=D0 → taken)
                         * Wait: CMP #$D0 / D0 04: BNE taken if != $D0. Since speed_val was loaded
                         * from table and we compare with $D0 again... speed_val may not be $D0.
                         * This sub-branch: if PLA'd A == $D0 → LDA #$08, BRA; else ORA #$20+STA+LDA #$08+BRA
                         * The PLA'd A IS speed_val (the ROM table value), not efc5.
                         * If speed_val == 0xD0: BNE nt → LDA_imm(16)+BRA(22) = 38 */
                        cy += 38;
                    } else {
                        /* speed_val != $D0: BNE_t(22)+LDA_abs_X(38)+ORA_imm(16)+STA_abs_X(38)+LDA_imm(16)+BRA(22) = 152 */
                        cy += 22+38+16+38+16+22;
                    }
                    goto dbb5_cyc;
                }
            }

d0e0_to_db60:
            {
                /* DB60: PHX(30)+LDA_dp(24)+TAX(14)+LDA_abs_X(38) = 106
                 * (no ASL×2 — ROM bytes: DA A5 47 AA BD AF F0 ... PHX LDA_dp TAX LDA_abs_X)
                 * DB67: F0 03 = BEQ +3 → target DB6C (f0af==0) or fall through DB69 (f0af!=0)
                 */
                uint8_t f0af_v = snes->ram[0xF0AF + mon47];
                cy += 30+24+14+38;  /* PHX+LDA_dp+TAX+LDA_abs_X = 106 */
                if (!f0af_v) {
                    /* BEQ_t(22)+PLX(36)+BRA(22)+RTS(42) = 122 → DBD9=RTS */
                    cy += 22+36+22+42;
                    goto dafe_cyc_done;
                }
                /* f0af != 0: BEQ_nt(16)+PLX(36)+BRA(22) → DB6F = 74 */
                cy += 16+36+22;
                /* DB6F: LDA $EFC5,X(38)+CMP #$C0(16)+BEQ → DBD9=RTS if ==C0 */
                if (efc5 == 0xC0) {
                    cy += 38+16+22+42;  /* LDA_abs_X+CMP+BEQ_t+RTS = 118 */
                    goto dafe_cyc_done;
                }
                /* efc5 != C0: BEQ_nt(16) → DB76 */
                cy += 38+16+16;  /* LDA_abs_X+CMP+BEQ_nt = 70 */
                /* DB76: PHA(8)+LDA_abs_X(38)+AND_imm(16)+STA_abs_X(38)+PLA(8)+STZ_abs_X(34)+STZ_abs_X(34)+CMP_imm(16) = 192 */
                cy += 8+38+16+38+8+34+34+16;
                if (efc5 == 0xB0) {
                    /* BNE_nt(16)+LDA_abs_X(38)+ORA_imm(16)+STA_abs_X(38)+LDA_imm(16)+BRA(22) = 146 */
                    cy += 16+38+16+38+16+22;
                } else {
                    /* BNE_t(22)+skip_block → LDA_abs_X(38)+ORA(16)+STA(38)+LDA_imm(16)+BRA(22)... approximate */
                    cy += 22+38+16+38+16+22;  /* same length, BNE target reaches same tail */
                }
                cy += 38+16+38+16+38+14;  /* SEC+SBC+STA_abs_X+CMP+LDA_abs_X+TAX approx */
                goto dbb5_cyc;
            }

dc23_cyc:
            {
                uint16_t cal_y2 = snes->cpu->y;
                uint8_t a30 = snes->ram[0xF015 + cal_y2] & 0x30;
                cy += 38+16;  /* LDA_abs_Y+AND_imm = 54 */
                if (a30 == 0) {
                    cy += 22+38+16+38+42;  /* BEQ_t→DC3F+ORA+STA+RTS */
                } else if (a30 & 0x20) {
                    cy += 16+16+38+16+38+38+16+38+22+42;  /* full path */
                } else {
                    cy += 16+16+22+38+14+38+22+42; /* bit4 only */
                }
                goto dafe_cyc_done;
            }

dbb5_cyc:
            {
                uint16_t cal_y2 = snes->cpu->y;
                uint8_t a30 = snes->ram[0xF015 + cal_y2] & 0x30;
                /* LDA_abs_Y(38)+AND_imm(16)+... */
                if (a30 == 0) {
                    cy += 38+16+22+38+16+38+42;  /* BEQ_t+ORA+STA+RTS = 76+134 */
                } else if (a30 & 0x20) {
                    cy += 38+16+16+16+38+16+38+38+14+38+22+42; /* bit5 path + double */
                } else {
                    cy += 38+16+16+16+22+38+14+38+22+42; /* bit4 only */
                }
                goto dafe_cyc_done;
            }
        }

dafe_cyc_done:;
        inject_cycles(snes, cy - 16);
    }
}

// SPIKE_COMPARE: region
// CONTRACT:
//   inputs_reg:  y=8
//   inputs_ram:  0xEFC5=1, 0xF0AF=1, 0xF014=1, 0xEFCE=1, 0xEFCF=1, 0xEFD1=1, 0xEFD3=1
//   output_ram:  0xEFCE=1
//   entry_mode:  mf=true, xf=false, dp=0x0, db=0x7E
//   entry_flags: z=auto, n=auto
// REVERSED_FUNCTION: btlgfx::InitMonsterAnim ($02:DAFE)
//
// COVERAGE NOTE (why these exact inputs). X and $47 are left undeclared (=0)
// so every indexed access ($EFC5+X, $F0AF+$47, $F015+$47*4, ...) lands on a
// byte the fuzz controls, not on a scattered zeroed cell — the routine's logic
// is slot-independent (mon_x=(uint8_t)X is trivially exact for the 0..5 slot
// range), so slot-0 coverage proves the branch logic. The two prologue GUARDS
// ($F015 & 0xC0, $F2BC) are also left undeclared (=0 baseline) on purpose: a
// randomized $F2BC is non-zero ~255/256 and would force the early-exit every
// trial, leaving the body unreached (verified: a deliberate write mutation
// went undetected under that config). With the guards passing, a randomized
// $F0AF (non-zero) drives the DB6F write path and $EFC5 exercises the
// D0/E0/C0/B0 sentinels — a write mutation in that path IS caught. The
// attr30!=0 sub-branches ($F015,Y bits 5-4) and the multi-slot indexed
// arithmetic stay beyond this fuzz; they were covered by the whole-game oracle
// (savestates 002/005/006, 0 diff — MemPalace ff4-port-frame26-debug).
