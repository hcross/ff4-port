/* FF4 desktop validation host — M3 A/B differential oracle.
 *
 * See docs/adr/0001-desktop-validation-in-re-loop.md and KNOWN_FINDINGS.md.
 *
 * Runs the SAME binary and the SAME post-seed state twice:
 *   - pass A: ff4_dispatch_enabled = 1  → native dispatch (the ported C
 *             routines, exactly as the G&W device runs them).
 *   - pass B: ff4_dispatch_enabled = 0  → pure LakeSnes interpretation of the
 *             original SNES code = GROUND TRUTH.
 * It captures a per-frame CRC of WRAM and of the blitted framebuffer for each
 * pass, then reports the FIRST frame at which they diverge, with the program
 * counter of both passes around that frame. That frame localises the buggy
 * ported hook for the RE loop to fix.
 *
 * The device compiles LakeSnes with FF4_PORT_STATIC_SNES — a single static
 * Snes singleton. Two live instances are therefore impossible; the oracle
 * snapshots one instance (snes_saveState into RAM) and restores it
 * (snes_loadState) between the two passes. ff4_dispatch_enabled is a plain
 * global, OUTSIDE the savestate, so the toggle survives the restore.
 *
 * Determinism self-test (--selftest): runs pass A as ON-vs-ON. A faithful
 * snapshot/restore + a port that keeps NO state outside the Snes must yield
 * zero divergence. Any divergence here means the oracle's own mechanism is
 * unsound (C-side static state, lossy savestate) and the A/B verdict cannot
 * be trusted until it is fixed. Always green-light the self-test on a seed
 * before believing its A/B result.
 *
 * Anti-hang (lesson F2): the coarse wall-clock timeout used in M2a is
 * inadequate. This oracle bounds nothing yet for the per-frame step (it runs
 * full ff4_step frames, fine for the healthy menu/scene seeds); when pointed
 * at the combat seed it must switch to a cycle-bounded probe (snes_runCycles)
 * — tracked as the M3-combat extension, gated on a trustworthy combat seed.
 *
 * Usage:
 *   ff4-desktop-oracle <rom.sfc> [flags]
 *     --frames N      compare N frames (default 600)
 *     --load f.lss    load a savestate seed after init (the scene to test)
 *     --selftest      ON-vs-ON determinism check instead of A/B
 *     --report f.txt  also write the verdict to a file
 *     --fb-only       compare framebuffer only (skip WRAM channel)
 *     --wram-only     compare WRAM only (skip framebuffer channel)
 *     --exclude PC    force a hook (hex original pc) to pure interpretation in
 *                     BOTH passes; repeatable. Use it to neutralise the
 *                     intentionally-divergent routines so the remaining
 *                     faithful ports can be checked. Clean baseline for the
 *                     healthy seeds: --exclude 15cadc --exclude 048004
 *                     (F3 OAM-DMA bypass, F4 ExecSound stub — see KNOWN_FINDINGS).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "snes/snes.h"   /* Snes, snes_loadState/saveState */
#include "snes/ppu.h"    /* Ppu, oam/highOam (for the OAM channel) */

/* ff4-gnw glue (ff4-gnw/main.c) */
extern bool ff4_init(const uint8_t *rom_bytes, int rom_length);
extern void ff4_step(void);
extern void ff4_shutdown(void);
extern void ff4_blit_to_lcd(uint16_t *lcd_fb);
extern Snes *ff4_snes;

/* dispatch counters + runtime toggle + trace hook (ff4-gnw/dispatch_all.c) */
extern uint32_t ff4_dispatch_hits;
extern uint32_t ff4_dispatch_misses;
extern int ff4_dispatch_enabled;
extern void (*ff4_dispatch_trace)(uint32_t pc);
extern int  (*ff4_dispatch_filter)(uint32_t pc);

/* --exclude set: hooks forced to pure interpretation in BOTH passes, so an
 * intentionally-divergent routine stops contaminating the comparison. */
#define EXCL_CAP 64
static uint32_t g_excl[EXCL_CAP];
static int      g_excl_n = 0;

static int dispatch_filter(uint32_t pc) {
    for (int i = 0; i < g_excl_n; i++) if (g_excl[i] == pc) return 0;
    return 1;
}

/* Per-hit dispatch trace: flat event log of (frame, original-pc) appended by
 * trace_cb during pass A, used to attribute the first diverging frame to the
 * concrete hook(s) that fired in it. */
#define TRACE_CAP 262144
static int      g_trace_frame  = 0;
static int      g_trace_active = 0;
static int      g_ev_n         = 0;
static int      g_ev_frame[TRACE_CAP];
static uint32_t g_ev_pc[TRACE_CAP];

static void trace_cb(uint32_t pc) {
    if (!g_trace_active || g_ev_n >= TRACE_CAP) return;
    g_ev_frame[g_ev_n] = g_trace_frame;
    g_ev_pc[g_ev_n]    = pc;
    g_ev_n++;
}

/* Print the distinct hooks that fired in frame `f` (dedup, in first-seen
 * order). pc → routine name is a grep away in ff4-gnw/dispatch_all.c. */
static void print_hooks_for_frame(FILE *o, int f) {
    uint32_t seen[256]; int sn = 0;
    int any = 0;
    for (int i = 0; i < g_ev_n; i++) {
        if (g_ev_frame[i] != f) continue;
        int dup = 0;
        for (int j = 0; j < sn; j++) if (seen[j] == g_ev_pc[i]) { dup = 1; break; }
        if (dup) continue;
        if (sn < 256) seen[sn++] = g_ev_pc[i];
        fprintf(o, "%s%06X", any ? ", " : "", g_ev_pc[i]);
        any = 1;
    }
    if (!any) fprintf(o, "(none — divergence is downstream of an earlier frame's hook)");
    fprintf(o, "\n");
}

/* Every distinct hook fired in frames [0, upto], first-seen order — the suspect
 * set when a divergence/hang has no hook in its own frame. */
static void print_suspects_upto(FILE *o, int upto) {
    uint32_t seen[256]; int sn = 0, any = 0;
    for (int i = 0; i < g_ev_n; i++) {
        if (g_ev_frame[i] > upto) continue;
        int dup = 0;
        for (int j = 0; j < sn; j++) if (seen[j] == g_ev_pc[i]) { dup = 1; break; }
        if (dup) continue;
        if (sn < 256) seen[sn++] = g_ev_pc[i];
        fprintf(o, "%s%06X", any ? ", " : "", g_ev_pc[i]); any = 1;
    }
    fprintf(o, any ? "\n" : "(none)\n");
}

#define LCD_W 320
#define LCD_H 240

static uint32_t crc32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++)
            crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)(-(int32_t)(crc & 1)));
    }
    return ~crc;
}

/* FF4's CPU stack: 00/8026 LDX #$02FF / TXS — top $02FF, grows down into pages
 * 1-2. Bytes below the live SP are dead scratch; a ported routine that sets
 * cpu->db/dp directly (skipping the asm's pha/phx/pld) leaves different scratch
 * there. Per the asm→C methodology (§2 ignore-list) the stack region is masked
 * in the per-frame compare so that scratch is not a false divergence. */
#define STACK_LO 0x0100u
#define STACK_HI 0x0300u

/* crc32 over WRAM with [STACK_LO,STACK_HI) treated as zero (stack masked). */
static uint32_t crc32_wram(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++) {
        uint8_t b = (i >= STACK_LO && i < STACK_HI) ? 0 : data[i];
        crc ^= b;
        for (int k = 0; k < 8; k++)
            crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)(-(int32_t)(crc & 1)));
    }
    return ~crc;
}

static uint8_t *read_file(const char *path, long *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n <= 0) { fclose(f); return NULL; }
    uint8_t *buf = malloc((size_t)n);
    if (!buf || fread(buf, 1, (size_t)n, f) != (size_t)n) { free(buf); fclose(f); return NULL; }
    fclose(f);
    *out_len = n;
    return buf;
}

/* One frame's observable fingerprint. */
typedef struct {
    uint32_t wram;     /* crc32 of the 128 KB WRAM */
    uint32_t fb;       /* crc32 of the blitted RGB565 framebuffer */
    uint32_t oam;      /* crc32 of PPU OAM (oam[0x100] + highOam[0x20] = 544 B) */
    uint8_t  k;        /* program bank after the frame */
    uint16_t pc;       /* program counter after the frame */
    uint64_t cycles;   /* cumulative cpu cycles (delta exposes a stuck frame) */
} FrameHash;

/* crc32 over the two non-contiguous OAM tables, chained. */
static uint32_t oam_crc(Snes *snes) {
    uint32_t c = crc32((const uint8_t *)snes->ppu->oam, sizeof(snes->ppu->oam));
    /* fold highOam in by continuing from c's complement (cheap chain). */
    uint32_t h = crc32(snes->ppu->highOam, sizeof(snes->ppu->highOam));
    return c ^ (h * 0x9E3779B1u);
}

/* Byte-accurate WRAM divergence (authoritative; immune to the per-frame CRC's
 * cycle-skew false positives). Pass A stores its per-frame WRAM into g_snapA;
 * pass B compares its live WRAM against it, stack-masked, and records the first
 * truly-divergent frame and the offending offsets. */
#define WRAM_SZ 0x20000
static uint8_t (*g_snapA)[WRAM_SZ] = 0;
static int      g_snap_cap   = 0;     /* # frames g_snapA can hold */
static int      g_byte_first = -1;    /* first byte-divergent frame (B vs A), or -1 */
static uint32_t g_byte_off[16];
static int      g_byte_noff  = 0;

static int wram_first_diff(const uint8_t *a, const uint8_t *b) {
    for (int i = 0; i < WRAM_SZ; i++) {
        if (i >= STACK_LO && i < STACK_HI) continue;  /* mask CPU stack scratch */
        if (a[i] != b[i]) return i;
    }
    return -1;
}

/* Run up to `frames` cycle-bounded frames. Returns the frame index at which the
 * bounded runner stalled (CPU-opcode budget exhausted inside one frame = a
 * hang), or -1 if all frames completed. The stalled frame's hashes ARE recorded
 * (the mid-hang state) before returning. If `snap` is set: store==1 copies WRAM
 * per frame (pass A); store==0 byte-compares against snap[i] (pass B). */
static int run_pass(int dispatch_enabled, int frames, FrameHash *out, int trace,
                    uint64_t budget, uint8_t (*snap)[WRAM_SZ], int store) {
    ff4_dispatch_enabled = dispatch_enabled;
    g_trace_active = trace;
    static uint16_t fb[LCD_W * LCD_H];
    for (int i = 0; i < frames; i++) {
        g_trace_frame = i;
        bool done = snes_runFrameBounded(ff4_snes, budget);
        memset(fb, 0, sizeof(fb));
        ff4_blit_to_lcd(fb);
        out[i].wram   = crc32_wram(ff4_snes->ram, sizeof(ff4_snes->ram));
        out[i].fb     = crc32((const uint8_t *)fb, sizeof(fb));
        out[i].oam    = oam_crc(ff4_snes);
        out[i].k      = ff4_snes->cpu->k;
        out[i].pc     = ff4_snes->cpu->pc;
        out[i].cycles = ff4_snes->cycles;
        if (snap && i < g_snap_cap) {
            if (store) {
                memcpy(snap[i], ff4_snes->ram, WRAM_SZ);
            } else if (g_byte_first < 0) {
                int off = wram_first_diff(snap[i], ff4_snes->ram);
                if (off >= 0) {
                    g_byte_first = i;
                    for (int o = off; o < WRAM_SZ && g_byte_noff < 16; o++) {
                        if (o >= STACK_LO && o < STACK_HI) continue;
                        if (snap[i][o] != ff4_snes->ram[o]) g_byte_off[g_byte_noff++] = (uint32_t)o;
                    }
                }
            }
        }
        if (!done) return i;   /* budget exhausted: stall/hang at frame i */
    }
    return -1;
}

/* Snapshot the live Snes into a freshly malloc'd buffer; caller frees. */
static uint8_t *snapshot(int *out_size) {
    int sz = snes_saveState(ff4_snes, NULL);
    uint8_t *buf = malloc((size_t)sz);
    if (buf) snes_saveState(ff4_snes, buf);
    *out_size = sz;
    return buf;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,
            "usage: %s <rom.sfc> [--frames N] [--budget OPS] [--load f.lss]"
            " [--selftest] [--report f.txt] [--fb-only|--wram-only] [--exclude PC]\n", argv[0]);
        return 2;
    }
    const char *rom_path = argv[1];
    int frames = 600;
    uint64_t budget = 4000000;   /* CPU-opcode budget per frame; a hang trips it */
    const char *load_path = NULL, *report_path = NULL;
    bool selftest = false, cmp_wram = true, cmp_fb = true;

    for (int i = 2; i < argc; i++) {
        if      (!strcmp(argv[i], "--frames") && i + 1 < argc) frames = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--budget") && i + 1 < argc) budget = strtoull(argv[++i], NULL, 0);
        else if (!strcmp(argv[i], "--load")   && i + 1 < argc) load_path = argv[++i];
        else if (!strcmp(argv[i], "--report") && i + 1 < argc) report_path = argv[++i];
        else if (!strcmp(argv[i], "--selftest")) selftest = true;
        else if (!strcmp(argv[i], "--fb-only"))   cmp_wram = false;
        else if (!strcmp(argv[i], "--wram-only")) cmp_fb = false;
        else if (!strcmp(argv[i], "--exclude") && i + 1 < argc) {
            if (g_excl_n < EXCL_CAP) g_excl[g_excl_n++] = (uint32_t)strtoul(argv[++i], NULL, 16);
        }
        else { fprintf(stderr, "error: bad arg '%s'\n", argv[i]); return 2; }
    }
    if (frames <= 0) { fprintf(stderr, "error: --frames must be > 0\n"); return 2; }

    long rom_len = 0;
    uint8_t *rom = read_file(rom_path, &rom_len);
    if (!rom) { fprintf(stderr, "error: cannot read ROM '%s'\n", rom_path); return 1; }

    if (!ff4_init(rom, (int)rom_len)) {
        fprintf(stderr, "error: ff4_init failed\n"); free(rom); return 1;
    }
    printf("ff4_init ok: rom=%ld bytes\n", rom_len);

    if (load_path) {
        long st_len = 0;
        uint8_t *st = read_file(load_path, &st_len);
        if (!st) { fprintf(stderr, "error: cannot read state '%s'\n", load_path); free(rom); return 1; }
        bool ok = snes_loadState(ff4_snes, st, (int)st_len);
        free(st);
        if (!ok) { fprintf(stderr, "error: snes_loadState rejected '%s' (%ld bytes)\n", load_path, st_len); free(rom); return 1; }
        printf("loaded seed   : %s (%ld bytes) | pc=%02X:%04X\n",
               load_path, st_len, ff4_snes->cpu->k, ff4_snes->cpu->pc);
    }

    /* Install the per-hook exclusion filter (applies to BOTH passes, so an
     * excluded routine is interpreted identically on each side → contributes
     * zero divergence). */
    if (g_excl_n > 0) {
        ff4_dispatch_filter = dispatch_filter;
        printf("excluded hooks: ");
        for (int i = 0; i < g_excl_n; i++) printf("%s%06X", i ? ", " : "", g_excl[i]);
        printf("  (forced to pure interpretation in both passes)\n");
    }

    /* Snapshot the common starting point S0 (post-init, post-seed). */
    int s0_size = 0;
    uint8_t *s0 = snapshot(&s0_size);
    if (!s0) { fprintf(stderr, "error: snapshot failed\n"); free(rom); return 1; }
    printf("snapshot S0   : %d bytes | pc=%02X:%04X\n",
           s0_size, ff4_snes->cpu->k, ff4_snes->cpu->pc);

    FrameHash *A = calloc((size_t)frames, sizeof(FrameHash));
    FrameHash *B = calloc((size_t)frames, sizeof(FrameHash));
    if (!A || !B) { fprintf(stderr, "error: oom\n"); return 1; }

    /* Pass A: native dispatch (= device). Trace the fired hooks for later
     * attribution of the diverging frame. */
    int sideA = 1;
    int sideB = selftest ? 1 : 0;     /* self-test compares ON vs ON */

    /* Byte-accurate WRAM snapshots for pass A (authoritative divergence). Cap
     * the buffer so memory stays bounded; beyond the cap we fall back to CRC. */
    g_snap_cap = frames < 512 ? frames : 512;
    g_snapA = malloc((size_t)g_snap_cap * WRAM_SZ);
    if (!g_snapA) g_snap_cap = 0;     /* OOM → CRC-only, still works */

    ff4_dispatch_trace = trace_cb;
    g_ev_n = 0;
    printf("\npass A: dispatch=%s, %d frames (budget %llu ops/frame)...\n",
           sideA ? "ON" : "OFF", frames, (unsigned long long)budget);
    int stallA = run_pass(sideA, frames, A, /*trace=*/1, budget, g_snapA, /*store=*/1);
    uint32_t hitsA = ff4_dispatch_hits, missA = ff4_dispatch_misses;
    ff4_dispatch_trace = 0;           /* pass B must not append events */
    if (stallA >= 0) printf("  ⚠ pass A STALLED at frame %d (budget exhausted)\n", stallA);

    /* Restore S0 and run pass B on the identical starting state. */
    if (!snes_loadState(ff4_snes, s0, s0_size)) {
        fprintf(stderr, "error: snes_loadState(S0) failed on restore\n"); return 1;
    }
    ff4_dispatch_hits = ff4_dispatch_misses = 0;
    printf("pass B: dispatch=%s, %d frames...%s\n",
           sideB ? "ON" : "OFF", frames, selftest ? "  (SELF-TEST)" : "");
    int stallB = run_pass(sideB, frames, B, /*trace=*/0, budget, g_snapA, /*store=*/0);
    uint32_t hitsB = ff4_dispatch_hits, missB = ff4_dispatch_misses;
    if (stallB >= 0) printf("  ⚠ pass B STALLED at frame %d (budget exhausted)\n", stallB);

    /* Compare frame by frame. Track first WRAM and first FB divergence
     * separately: WRAM-only divergence may be internal scratch, while a FB
     * divergence is a gameplay-visible difference against ground truth. */
    /* Compare only frames both passes fully completed: a stalled frame's hashes
     * are mid-hang state, not a meaningful comparison point. */
    int cmp_lim = frames;
    if (stallA >= 0 && stallA < cmp_lim) cmp_lim = stallA;
    if (stallB >= 0 && stallB < cmp_lim) cmp_lim = stallB;

    int first = -1, first_wram = -1, first_fb = -1, first_oam = -1;
    const char *channel = "";
    for (int i = 0; i < cmp_lim; i++) {
        bool w = cmp_wram && (A[i].wram != B[i].wram);
        bool f = cmp_fb   && (A[i].fb   != B[i].fb);
        bool m = (A[i].oam != B[i].oam);
        if (w && first_wram < 0) first_wram = i;
        if (f && first_fb   < 0) first_fb   = i;
        if (m && first_oam  < 0) first_oam  = i;
        if ((w || f) && first < 0) {
            first = i;
            channel = (w && f) ? "WRAM+FB" : (w ? "WRAM" : "FB");
        }
    }

    bool native_hang = (stallA >= 0) && (stallB < 0 || stallB > stallA);

    /* Verdict. */
    char buf[4096];
    int n = 0;
    n += snprintf(buf + n, sizeof(buf) - n, "\n=== A/B oracle verdict ===\n");
    n += snprintf(buf + n, sizeof(buf) - n, "mode          : %s\n",
                  selftest ? "SELF-TEST (ON vs ON, expect IDENTICAL)" : "A/B (dispatch ON vs pure interpreter)");
    n += snprintf(buf + n, sizeof(buf) - n, "seed          : %s\n", load_path ? load_path : "(boot, no seed)");
    n += snprintf(buf + n, sizeof(buf) - n, "frames        : %d\n", frames);
    n += snprintf(buf + n, sizeof(buf) - n, "channels      : %s%s%s\n",
                  cmp_wram ? "WRAM " : "", cmp_fb ? "FB" : "", (cmp_wram||cmp_fb)?"":"(none!)");
    n += snprintf(buf + n, sizeof(buf) - n, "dispatch A    : %u hits / %u miss\n", hitsA, missA);
    n += snprintf(buf + n, sizeof(buf) - n, "dispatch B    : %u hits / %u miss\n", hitsB, missB);
    n += snprintf(buf + n, sizeof(buf) - n, "compared      : %d / %d frames%s\n",
                  cmp_lim, frames, cmp_lim < frames ? " (truncated by a stall)" : "");
    if (stallA >= 0)
        n += snprintf(buf + n, sizeof(buf) - n,
            "stall A (nat) : frame %d — CPU-opcode budget (%llu) exhausted inside one frame = HANG\n",
            stallA, (unsigned long long)budget);
    if (stallB >= 0)
        n += snprintf(buf + n, sizeof(buf) - n,
            "stall B (int) : frame %d — interpreter also stalled (suspect seed/harness, not the port)\n", stallB);

    if (first < 0 && native_hang) {
        n += snprintf(buf + n, sizeof(buf) - n,
            "RESULT        : NATIVE HANG at frame %d (no functional divergence before it).\n", stallA);
        n += snprintf(buf + n, sizeof(buf) - n,
            "              → native dispatch hangs where the interpreter runs on; the culprit hook is\n"
            "                among those that fired up to this frame.\n");
    } else if (first < 0) {
        n += snprintf(buf + n, sizeof(buf) - n, "RESULT        : IDENTICAL across %d frames%s.\n",
                      cmp_lim, (stallA>=0||stallB>=0) ? " compared" : "");
        if (selftest)
            n += snprintf(buf + n, sizeof(buf) - n,
                "              → snapshot/restore is deterministic; oracle mechanism TRUSTED for this seed.\n");
        else
            n += snprintf(buf + n, sizeof(buf) - n,
                "              → ported routines match ground truth over this window (no divergence to localise).\n");
    } else {
        n += snprintf(buf + n, sizeof(buf) - n,
            "RESULT        : DIVERGENCE at frame %d (channel: %s)\n", first, channel);
        if (selftest)
            n += snprintf(buf + n, sizeof(buf) - n,
                "              → ⚠ SELF-TEST FAILED: snapshot/restore is NOT deterministic.\n"
                "                The port holds state outside the Snes, or savestate is lossy.\n"
                "                The A/B verdict CANNOT be trusted until this is fixed.\n");
        else
            n += snprintf(buf + n, sizeof(buf) - n,
                "              → first frame where native dispatch parts from ground truth.\n");
        n += snprintf(buf + n, sizeof(buf) - n,
            "  first WRAM div: frame %d\n", first_wram);
        n += snprintf(buf + n, sizeof(buf) - n,
            "  first FB   div: %s%d\n",
            first_fb < 0 ? "(none — output matches ground truth) " : "frame ", first_fb);
        n += snprintf(buf + n, sizeof(buf) - n,
            "  first OAM  div: %s%d\n",
            first_oam < 0 ? "(none — OAM matches ground truth) " : "frame ", first_oam);
        if (g_snap_cap > 0) {
            if (g_byte_first < 0)
                n += snprintf(buf + n, sizeof(buf) - n,
                    "  WRAM bytes    : IDENTICAL (CRC 'WRAM div' above is a cycle-skew false positive)\n");
            else {
                n += snprintf(buf + n, sizeof(buf) - n,
                    "  WRAM bytes    : first real divergence frame %d (authoritative, byte-exact):", g_byte_first);
                for (int k = 0; k < g_byte_noff; k++)
                    n += snprintf(buf + n, sizeof(buf) - n, " $%05X", g_byte_off[k]);
                n += snprintf(buf + n, sizeof(buf) - n, "%s\n", g_byte_noff >= 16 ? " …" : "");
            }
        }
        int p = first > 0 ? first - 1 : first;
        n += snprintf(buf + n, sizeof(buf) - n,
            "  frame %-4d  : A pc=%02X:%04X wram=%08X fb=%08X | B pc=%02X:%04X wram=%08X fb=%08X\n",
            p, A[p].k, A[p].pc, A[p].wram, A[p].fb, B[p].k, B[p].pc, B[p].wram, B[p].fb);
        n += snprintf(buf + n, sizeof(buf) - n,
            "  frame %-4d  : A pc=%02X:%04X wram=%08X fb=%08X | B pc=%02X:%04X wram=%08X fb=%08X  <-- diverged\n",
            first, A[first].k, A[first].pc, A[first].wram, A[first].fb,
            B[first].k, B[first].pc, B[first].wram, B[first].fb);
        long long dcyc = (long long)A[first].cycles - (long long)B[first].cycles;
        long long adcyc = dcyc < 0 ? -dcyc : dcyc;
        n += snprintf(buf + n, sizeof(buf) - n,
            "  cycles@%-4d : A=%llu  B=%llu  (delta %+lld — %s)\n",
            first, (unsigned long long)A[first].cycles, (unsigned long long)B[first].cycles,
            dcyc,
            adcyc > 1000 ? "large ⇒ control-flow/timing divergence"
                         : "small ⇒ localised functional/state divergence");
    }
    fputs(buf, stdout);

    /* Hook attribution (A/B only): which native routines fired in/just-before
     * the diverging frame. Use the byte-accurate frame (g_byte_first) when
     * available — the CRC `first` can be a cycle-skew false positive a frame
     * early, mis-attributing the culprit. */
    int att = (g_snap_cap > 0 && g_byte_first >= 0) ? g_byte_first : first;
    if (!selftest && g_snap_cap > 0 && g_byte_first < 0 && first >= 0) {
        printf("  (no byte-level WRAM divergence — skipping hook attribution; FB/OAM or skew only)\n");
    } else if (!selftest && att >= 0) {
        printf("  hooks @frame %-4d (diverged): ", att);
        print_hooks_for_frame(stdout, att);
        if (att > 0) {
            printf("  hooks @frame %-4d (prev)    : ", att - 1);
            print_hooks_for_frame(stdout, att - 1);
        }
        /* When the diverging frame has no local hook, the cause is an earlier
         * hook whose effect surfaced late: list every distinct hook fired in
         * [0, att] so the suspect set is explicit. */
        printf("  hooks in [0,%d] (suspects)  : ", att);
        print_suspects_upto(stdout, att);
        printf("  (pc → routine: grep the pc in ff4-gnw/dispatch_all.c)\n");
    } else if (!selftest && native_hang) {
        printf("  hooks @frame %-4d (hang)    : ", stallA);
        print_hooks_for_frame(stdout, stallA);
        printf("  hooks in [0,%d] (suspects)  : ", stallA);
        print_suspects_upto(stdout, stallA);
        printf("  (pc → routine: grep the pc in ff4-gnw/dispatch_all.c)\n");
    }

    if (report_path) {
        FILE *o = fopen(report_path, "w");
        if (o) {
            fputs(buf, o);
            if (!selftest && first >= 0) {
                fprintf(o, "  hooks @frame %d (diverged): ", first);
                print_hooks_for_frame(o, first);
                if (first > 0) {
                    fprintf(o, "  hooks @frame %d (prev)    : ", first - 1);
                    print_hooks_for_frame(o, first - 1);
                }
            }
            fclose(o);
            printf("report written: %s\n", report_path);
        }
        else fprintf(stderr, "warning: cannot write report '%s'\n", report_path);
    }

    int rc;
    if (selftest) rc = (first < 0) ? 0 : 3;   /* self-test: divergence is failure */
    else          rc = (first < 0) ? 0 : 1;   /* A/B: divergence found → 1 (signal) */

    free(A); free(B); free(s0); free(g_snapA);
    ff4_shutdown();
    free(rom);
    return rc;
}
