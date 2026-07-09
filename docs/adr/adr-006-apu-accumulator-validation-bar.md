# ADR-006 — Relax the byte-identical validation bar for the APU catchup accumulator only

- **Status**: Accepted 2026-07-09 (implementation pending)
- **Date**: 2026-07-09
- **Deciders**: Hoani Cross, claude-code
- **Scope**: `ff4-gnw/snes/snes.c` (`apuCatchupCycles` accumulation),
  the golden-sweep validation methodology (M1/M3a bar), future perf work
  on `snes_runCycles`

## Context

Core-engine surgery (the M1 line renderer, the M3a event-batched
`snes_runCycles`) is validated against a **byte-identical bar**: final PPM
+ WRAM dumps across coldboot + fixtures at two depths, plus per-frame
framebuffer CRCs over 600-frame runs, must match the pre-change build
exactly. This bar is what lets a large rewrite of hot code land with
confidence and it remains the project default.

That bar has one structurally expensive consequence. LakeSnes accumulates
APU catchup time in a `double` (`snes->apuCatchupCycles`), one addition of
a constant per 2-master-cycle tick. Floating-point addition is not
associative: `a += k` executed n times does not round identically to
`a += k * n`. So the M3a event batching — whose whole point is to stop
doing per-tick work — must still **replay one `double` addition per tick**
(~357k serial, dependency-chained additions per frame) purely to reproduce
the historical rounding sequence. D6 measurement on the Cortex-M7
(2026-07-09) puts the remaining emulation cost at 59.0 ms/frame; the
replay chain is estimated at 7–9 ms of it (~4–7 cycles latency per fp64
add at 280 MHz). Those milliseconds buy **no correctness** — only
bit-compatibility with the accumulated rounding error of the original
per-tick loop.

The underlying quantity is exactly rational: the APU/master clock ratio is
`(32040 × 32) / (1364 × 262 × 60)` (NTSC). An integer accumulator (e.g.
master-cycle numerator against that denominator) is *more* accurate than
the `double` it replaces, and O(1) per segment instead of O(ticks). But it
truncates to whole APU cycles at *slightly different instants* than the
fp history — occasionally ±1 SPC cycle at a catchup point — which shifts
APU-visible timing enough that WRAM/framebuffer baselines from fp builds
no longer match byte-for-byte. Under the default bar, that implementation
is unlandable no matter how correct it is.

## Decision

Relax the byte-identical bar **for the APU catchup accumulator only**. A
reformulation of `apuCatchupCycles` (fp → exact integer/rational) is
landable without byte-identity against pre-change baselines, provided ALL
of the following replacement evidence accompanies it:

1. **Bounded-deviation argument** (reviewed, in the commit message): the
   new accumulator's truncation instants differ from the fp history by at
   most 1 APU cycle at any catchup point, and the difference does not
   accumulate over time.
2. **Oracle verdicts unchanged**: `scripts/regress.sh` across all
   tractable fixtures — every fixture's A/B oracle verdict must be
   identical to its recorded baseline (divergence-free fixtures stay
   divergence-free; known-divergent fixtures diverge no earlier).
3. **Long-run stability**: the full golden sweep completes with equal
   `frames_run` everywhere, and the new build is **self-consistent**
   (running the sweep twice on the new build is byte-identical — the
   determinism property itself is preserved).
4. **Same-day device A/B** (the M3a methodology): flash old and new in
   identical conditions, read D6; no functional deviation (scene renders,
   liveness oracle passes), and the measured gain is stated with both
   numbers.
5. **Baselines regenerated**: golden baselines from fp-era builds are
   declared obsolete in the same change; the new build's sweep output
   becomes the reference for all subsequent byte-identical work.

## Consequences

- The byte-identical bar remains the **default** for all other core
  surgery. This ADR carves out exactly one accumulator, with a heavier
  evidence pack in exchange.
- After the switch, every future change goes back to plain byte-identity
  — measured against post-switch baselines.
- The 7–9 ms/frame currently spent replaying fp rounding on the M7
  becomes recoverable (next emulation-side lever after this ADR's
  implementation, alongside M2 spin fast-forward and continued dispatch).
- Savestate compatibility: `apuCatchupCycles` is serialized as a double
  in `.lss` states. The implementation must keep loading (and either keep
  writing, or version) that field so existing fixtures stay loadable.

## Alternatives considered

- **Keep the replay forever**: safe, but permanently taxes every frame to
  emulate rounding error; rejected as antithetical to the 60 fps goal.
- **Fused `a += k*n` without an ADR**: silently breaks the bar the rest of
  the project relies on; rejected.
- **Relax the bar globally** (e.g. "close enough" pixel/WRAM diffs):
  destroys the project's strongest regression tripwire for a one-site
  problem; rejected.
