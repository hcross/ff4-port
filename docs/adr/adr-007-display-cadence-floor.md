# ADR-007 — Display-cadence floor outranks game-clock exactness (no deeper frame skip)

- **Status**: Accepted 2026-07-14
- **Date**: 2026-07-14
- **Deciders**: Hoani Cross (two explicit on-device verdicts), claude-code
- **Scope**: the device frame-pacing loop (retro-go-sd
  `Core/Src/porting/ff4/main_ff4.c`, adaptive render skip), and — by
  consequence — the priority order of all future `ff4-gnw` rendering
  performance work. Filed in this repo because it is the project's ADR
  home (see ADR-002/ADR-006, which also constrain `ff4-gnw`).

## Context

The device renders slower than it emulates: after the 2026-07 render
campaigns, continuous scroll still costs well above the 16.7 ms/frame
that 60 fps requires (the Baron-castle-exterior bench decomposes to
~10.2 ms emulation + ~37 ms render per frame, 2026-07-14 measurement).
Skipping the *render* (never the emulation) of some frames is the
obvious lever, and two designs were built and put in front of the user
on the real device:

1. **Fixed frameskip** (`FF4_FRAMESKIP=2`, render 1 of 3, 2026-07-09):
   game speed improves, but the verdict was that "the display reads as a
   slideshow, not a game" (comment hardened into `main_ff4.c` at the
   time). Rejected.
2. **Adaptive skip v2 — steady odd period** (2026-07-14, retro-go-sd
   `21bfd6d1`): period escalates 1/3/5/7 under load, game clock stays an
   *exact* 60 Hz everywhere, display settles to a regular ~8.6 fps in
   the heavy zone. Verdict (translated from the session): "I don't like
   it at all — it looks like static frameskip." Rejected and reverted
   the same night (`32239edd`), restoring v1.

The accepted behavior is **adaptive skip v1** (retro-go-sd `9c58d2e0`,
user-validated 2026-07-13, "fluid everywhere" on his real walk at a
measured 47 % skip rate): skip at most every other render, no two
consecutive skips. Its 50 % cap means any zone whose rendered frame
exceeds ~33 ms runs in visible slow motion — and the user prefers that
slow motion to a lower-but-regular display cadence, *even when the
lower cadence comes with a perfect game clock*.

## Decision

The **display-cadence floor outranks game-clock exactness**. The v1
alternating skip (≤50 %, no two consecutive skips) is the ceiling of
acceptable render skipping on this project.

Do **not** re-propose deeper or steadier skipping — fixed-period,
adaptive, or otherwise, even with an exact 60 Hz game clock — without
Hoani explicitly reopening this decision.

## Consequences

- The **only** sanctioned path to fluidity in heavy zones is reducing
  the render cost itself (the R15–R19 renderer lineage, continued
  dispatch porting) until v1's cap suffices: render ≤ ~19 ms/frame lets
  v1 hold the 60 Hz game clock at ≥30 displayed fps.
- The "adaptive frame pacing" fallback lever in the umbrella's
  `PLAN-SUBFRAME.md` is closed; the plan's remaining levers are all
  render-cost levers.
- Perf sessions must budget for this: a zone that cannot be rendered
  cheaply enough stays slow — there is no pacing trick left to paper
  over it.

## Alternatives rejected

- **Fixed frameskip** — rejected by the user on device (2026-07-09,
  "slideshow"), kept only as a disabled tunable.
- **Adaptive steady-odd-period skip (v2)** — rejected by the user on
  device (2026-07-14) despite its exact game clock; the regular low
  display cadence is what reads as "static frameskip". Implementation
  preserved in retro-go-sd history (`21bfd6d1`) if ever reopened.

Session trail: MemPalace `wing=ff4-gnw`, `room=architecture-decisions`
(USER-VERDICT drawer, 2026-07-14) — this ADR is its promotion into the
versioned docs.
