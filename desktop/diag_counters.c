/* Shared weak definitions of the PPU diagnostic counters.
 *
 * ff4-gnw/snes/ppu.c increments these under #ifndef STM32H7B0xx (the device
 * build elides them), so EVERY desktop binary that links the core must
 * provide definitions. Historically only harness_headless.c did -- which
 * broke the (re)link of the oracle, wram_diff, input_probe, miss_profiler
 * and state_inject once the R17+ counters landed in ppu.c (found 2026-07-15
 * while wiring the translation-patch runtime; the stale pre-R17 binaries
 * had been masking it).
 *
 * Weak so harness_headless.c's strong, actively-reported definitions win in
 * the headless binary; everywhere else these zero-initialized fallbacks
 * satisfy the linker for counters the tool does not report. When ppu.c
 * grows a new counter, add it here (link failure of any non-headless tool
 * is the reminder). */

#define FF4_DIAG_WEAK __attribute__((weak))

FF4_DIAG_WEAK unsigned ff4_diag_trc_miss;
FF4_DIAG_WEAK unsigned ff4_diag_pal4_rebuild;
FF4_DIAG_WEAK unsigned ff4_diag_lr_slow_lines;
FF4_DIAG_WEAK unsigned ff4_diag_lr_math_lines;
