# ADR-001 — Native C port over a full software SNES emulator

- **Status**: Accepted
- **Date**: backfilled 2026-07-04 (the decision predates this document — it
  was the project's founding choice; see "Note on backfilled ADRs" below)
- **Deciders**: Hoani Cross
- **Scope**: `ff4` (project-wide), primarily affects `ff4-gnw` (the
  delivery artifact) and `ff4-port` (the methodology that produces it)

## Note on backfilled ADRs

This decision was made before this project adopted the ADR practice and is
reconstructed here from the project's own documented facts (`AGENTS.md`,
measured performance numbers already recorded in `ff4-gnw/README.md`), not
from a contemporaneous discussion log. Where a rationale below is inferred
rather than directly evidenced, it is worded as such.

## Context

The target device is a Nintendo Game & Watch (the `retro-go-sd` homebrew
firmware on an STM32H7B0VBT6) — a small in-order embedded microcontroller,
nothing like the desktop-class hardware SNES emulators are normally tuned
for. The project already has a working, cycle-level-accurate SNES
interpreter (LakeSnes, see ADR-002) that can run the original ROM
unmodified. The measured cost of doing *only* that — interpreting the full
CPU, PPU and APU in software on-device — is recorded in
[ADR 0001](0001-desktop-validation-in-re-loop.md): **~3.7-4 fps in PPU
Mode 1, ~6 fps in Mode 7**. FF4
needs a responsive 50/60 fps experience; interpretation alone does not get
there on this hardware.

## Decision

Keep the interpreter as the **execution engine and ground-truth reference**
(every routine must be provably equivalent to what the interpreter would
have done — see `AGENTS.md` §A.2), but progressively replace the hottest
and best-understood interpreted routines with **native, hand-verified C**
via a dispatch mechanism (`ff4_dispatch_try` in `dispatch_all.c`, `AGENTS.md`
§A.2): a JSR/JSL to a known SNES address is intercepted and a matching C
function runs instead of the interpreted asm. This is incremental — the
game runs correctly on day one (100% interpreted) and gets faster call by
call as native C absorbs the hot paths — rather than an all-or-nothing
rewrite.

## Consequences

### Positive
- The game is always playable, even mid-port: unported routines simply run
  interpreted, at correctness parity with the reference.
- Every native replacement has an oracle to prove equivalence against
  (the interpreter itself) — see the maturity ladder L0→L4 in `AGENTS.md`
  §B.2. There is no point at which "the C might not match the original."
- Performance gains are per-routine and measurable (dispatch hit rate,
  see `DISPATCH_REGISTRY.md`), not a single all-or-nothing bet.

### Negative
- Two parallel execution paths to keep coherent (native C vs interpreter)
  for the lifetime of the project, until (if ever) dispatch coverage
  reaches 100%.
- The dispatch mechanism itself has sharp edges that must be tracked
  project-wide (`AGENTS.md` §A.2's "Known dispatch limits": the WRAM
  write-hook is blind to direct C writes, `ff4_dispatch_try` doesn't charge
  the original routine's cycle cost, and JML/`$DC` is not intercepted at
  all — some routines must stay interpreted for that reason alone).

## Alternatives rejected

- **Ship the pure interpreter, optimize LakeSnes itself instead of porting
  routines.** Rejected on the measured numbers above: closing a ~10x
  performance gap (4-6 fps → 50-60 fps target) through emulator-core
  optimization alone, on an in-order MCU with slow external RAM, was
  judged far less tractable than replacing routines with native
  equivalents one at a time, each independently provable.
- **Rewrite the game logic from scratch in C without grounding it in the
  original assembly.** Rejected because it forfeits the one property this
  project depends on for correctness: an oracle to check against. A
  from-scratch reimplementation would be a fan remake, not a provably
  faithful port, and every subtle original behavior (including bugs
  players may rely on, per the `zelda3` project's experience with the same
  class of port) would have to be independently rediscovered.
- **Fully decompile the ROM to structured C, then compile that.** Not
  attempted: unlike compiler-generated binaries (where decompilers like
  Ghidra reconstruct high-level structure from a known compiler's
  patterns), FF4's 65816 assembly is hand-written, with no compiler
  fingerprint to exploit. `ca65-bridge`'s own capability table records
  `has_decompile: no — no pseudo-C` for exactly this reason.
