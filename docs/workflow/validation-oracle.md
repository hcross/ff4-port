# The validation oracle: proving a translation is actually correct

> If "oracle" or "shadow execution" are unfamiliar terms, read the
> umbrella primer first:
> [`03-reverse-engineering-and-shadow-execution.md`](https://github.com/hcross/ff4/blob/main/docs/primer/03-reverse-engineering-and-shadow-execution.md).
> This page is the concrete implementation of that idea in this repository.

A C candidate coming out of the translation pipeline
([`translation-cascade.md`](translation-cascade.md)) is a **guess** —
plausible-looking code, nothing more, until it's mechanically checked
against the original. This repository has two levels of checking, each
catching a different class of mistake.

## Level 1 — the per-routine spike

`translator/generate_spike.py` reads a candidate's `// CONTRACT:` block
and produces a small standalone program (`parity/src/spike__<addr>_*.c`)
that:

1. Sets up an identical CPU/memory starting state for both sides.
2. Runs the candidate C function.
3. Separately, runs the *original* assembly for the same routine, via
   `run_emulated_func()` — the original 65816 code, executed by the
   interpreter, from the same starting state.
4. Compares every output the CONTRACT declared (registers, RAM addresses,
   declared hardware effects).
5. Repeats this across many different (often fuzzed/randomized) input
   states, not just one hand-picked case.

A routine that passes this reaches maturity level **L2**. This is exactly
the "shadow execution" idea from the primer, scoped down to one routine in
isolation — which is both its strength (fast, cheap, easy to run
repeatedly) and its blind spot (see below).

## Why isolation isn't the whole story

A spike only compares what the CONTRACT *declares*. Two real, documented
failure modes fall through this gap:

- **An undeclared hardware side effect.** A routine that writes to a PPU
  or DMA register but only lists a WRAM address in its CONTRACT will pass
  its spike anyway — the spike never inspected the hardware bus, only
  the RAM locations it was told to check. This is Pitfall 16 in
  `prompts/pitfalls.yaml`, and it has shipped for real more than once.
- **Cross-routine interaction.** A routine that behaves correctly alone
  might still misbehave once surrounded by the actual game state that
  precedes and follows it in real play — state the isolated spike's
  synthetic starting point never reproduced.

## Level 2 — the whole-game A/B oracle

`desktop/` builds a full, playable instance of the game (linking the live
`ff4-gnw` tree directly, so it's always testing the code currently being
edited, not a stale copy) and runs two passes from the same starting
savestate:

- **Pass A ("dispatch ON")** — native C hooks active wherever available.
- **Pass B ("dispatch OFF", pure interpreter)** — the ground truth,
  exactly as if none of the porting work existed.

`wram_diff` / `oracle_ab.c` compare the two passes **frame by frame**,
across WRAM (and, depending on the tool, the framebuffer/VRAM), and report
the *first* point of divergence — which routine's dispatch caused the
game state to diverge from what the original would have done. This is the
mandatory gate documented in
[ADR 0001](../adr/0001-desktop-validation-in-re-loop.md): a routine is not
considered done because its spike is green, but because it survives real
gameplay under this A/B comparison. Passing this is what earns level **L3**.

## Why this runs on a desktop, not the device

The Game & Watch's STM32H7B0 is a small, in-order microcontroller with
comparatively slow external RAM — running this same A/B comparison
directly on-device, every time a routine changes, would be far too slow
for an iterative workflow. The desktop oracle establishes **correctness**
(does the game state agree, bit for bit) quickly; it cannot establish
**performance** (is it fast enough on the real, much slower hardware) —
that's a separate, final check, described in
[`ff4-gnw`'s own docs](https://github.com/hcross/ff4-gnw/tree/main/docs)
and the umbrella's [`workflows/WF-RELEASE.md`](https://github.com/hcross/ff4/blob/main/workflows/WF-RELEASE.md).
Desktop instruction counts and device wall-clock time are not the same
number — never assume one from the other.
