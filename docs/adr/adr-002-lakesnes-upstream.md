# ADR-002 — Use LakeSnes upstream, semantically unmodified

- **Status**: Accepted
- **Date**: backfilled 2026-07-04 (the decision predates this document —
  see "Note on backfilled ADRs" in ADR-001)
- **Deciders**: Hoani Cross
- **Scope**: `ff4-gnw` (the vendored `snes/` core), `ff4-port` (the
  desktop validation harness that links the same core)

## Context

Both the device build (`ff4-gnw`) and the validation harness
(`ff4-port/desktop/`, see ADR 0001) need a correctness reference: something
that runs the *original* 65816/PPU/APU behavior byte-for-byte, independent
of anything this project's own C hooks might get wrong. Writing a new SNES
core from scratch, or forking one and diverging it with FF4-specific
shortcuts, would each destroy that independence — a reference that has
absorbed the same assumptions as the thing it's supposed to check stops
being a reference. [LakeSnes](https://github.com/elzo-d/LakeSnes) (by
elzo-d) is a compact, readable C SNES emulator core; `zelda3`
(https://github.com/snesrev/zelda3, the sibling asm→C SNES port this
project's methodology follows) independently made the same choice for the
same reason.

## Decision

Vendor LakeSnes as the CPU/PPU/APU/DMA core (`ff4-gnw/snes/`) and treat its
**emulated behavior as untouchable**: no game-specific shortcuts, no
"close enough" approximations, patched directly into the core. The only
changes applied on top of upstream LakeSnes are purely mechanical,
target-integration patches, gated behind `#ifdef FF4_PORT_STATIC_SNES` so
they compile out for anyone using LakeSnes standalone:
- **Static allocation** (`-DFF4_PORT_STATIC_SNES`) instead of dynamic
  `malloc`, because the Game & Watch has no general-purpose heap large
  enough to assume.
- **ROM read XIP from external flash** instead of loading the whole ROM
  into RAM, because the on-device RAM budget doesn't have room for it.
- **The dispatch hook itself**, in `cpu.c`'s JSR/JSL cases — this is the
  interception point `ff4_dispatch_try` needs (see ADR-001), and is the
  one addition that is specific to this project's architecture rather than
  to the G&W target, but it does not change what any instruction computes.
- **Symbol prefixing** (`ff4_redefines`, `objcopy --redefine-syms`) so this
  project's copy of LakeSnes's globals don't collide with other homebrew
  overlays (`smw`, `zelda3`) also linked into `retro-go-sd`.

None of these change what the interpreter computes for a given ROM and
input — only how it's allocated, where it reads from, and where its
symbols live.

## Consequences

### Positive
- Every native-C replacement has a reference that is, by construction, not
  contaminated by this project's own assumptions.
- LakeSnes upstream fixes/improvements can in principle be pulled in
  without re-auditing this project's own hooks — the integration patches
  are additive and behind a flag.
- Matches the `zelda3` precedent directly: the same core, the same
  "reused HW emulator, kept semantically stock" pattern.

### Negative
- Upstream LakeSnes bugs or limitations are inherited as-is (e.g. it is
  not cycle-charging the SPC700 mailbox exactly the way the real hardware
  does in every corner case — a documented source of some pitfalls, see
  `prompts/pitfalls.yaml` Pitfall 13-15's MMIO/DMA notes).
- The static-allocation and symbol-prefixing patches must be re-applied
  and re-verified against any future LakeSnes upstream merge; there is no
  automated patch-tracking today.

## Alternatives rejected

- **Write a project-specific SNES core.** Rejected: reimplementing a
  mature emulator core for a single game's needs carries a high risk of
  introducing exactly the kind of subtle behavioral bugs the reference is
  supposed to catch, with no independent oracle left to validate it
  against.
- **Fork LakeSnes and add FF4-specific behavior directly into the core.**
  Rejected per `AGENTS.md` §A.2: LakeSnes's whole value here is being the
  **ground-truth reference** that every ported routine is checked against.
  Patching game-specific behavior into it would make the reference agree
  with the port by construction, defeating the entire validation
  methodology.
- **Evaluate other emulator cores.** Not part of this project's own
  history as far as its documentation records — LakeSnes was the
  incumbent choice from the project's inception, chosen (as best
  reconstructed from `AGENTS.md`) for being compact, readable C rather
  than being benchmarked against alternatives. Recorded here as an
  honestly-unevaluated alternative, not a rejected one.
