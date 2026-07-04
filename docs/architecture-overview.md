# Architecture overview

> New to reverse engineering, 65816 assembly, or SNES hardware? Read the
> umbrella repo's primer first:
> [`ff4/docs/primer/`](https://github.com/hcross/ff4/tree/main/docs/primer).
> This page assumes that background and focuses on how *this* repository
> is organized.

`ff4-port` is the **workshop**: everything needed to turn the original
Final Fantasy IV assembly into proven-equivalent C, but nothing that ships
on the device — that's [`ff4-gnw`](https://github.com/hcross/ff4-gnw)'s
job. Concretely, the repository is organized around one pipeline, split
into directories by pipeline stage:

```
upstream/        the disassembly + the ROM (the input to everything else)
   ↓
ca65-bridge/     reads the disassembly, classifies each routine
   ↓
translator/      LLM-assisted asm -> C translation pipeline
   ↓
port/            translation candidates in flight (not yet trusted)
   ↓
parity/          per-routine equivalence proof ("the spike")
   ↓
desktop/         whole-game validation oracle (A/B comparison)
   ↓
  (promoted candidates copied into ff4-gnw/<domain>/)
```

## `upstream/` — the ground truth

A git submodule holding the community disassembly
(`upstream/notes/ff4j-sfc.asm`) and the ROM itself (`upstream/rom/`, not
committed — see the repo's Quick Start for how to provide your own legally
obtained copy). Every other directory ultimately reads from here; nothing
in this repository invents game behavior that isn't traceable back to
these two sources.

## `ca65-bridge/` — reading the disassembly programmatically

A small Python tool that extracts a routine's assembly text by label,
finds what it calls and what calls it, and runs the translate-vs-delegate
classification (see [ADR-003](adr/adr-003-classification.md)) that decides
whether a routine is a good candidate for full C translation or should
just delegate to the interpreter. Nothing here modifies game behavior —
it's read-only tooling over `upstream/`.

## `translator/` — turning assembly into C candidates

Where the actual asm→C translation happens, LLM-assisted. Explained in
detail, with the reasoning behind each stage, in
[`docs/workflow/translation-cascade.md`](workflow/translation-cascade.md).
Output lands in `port/<module>/<func>.c` — a **candidate**, not yet
trusted.

## `parity/` — proving a candidate is correct in isolation

Generates and runs the **spike** for a candidate (see the umbrella
primer's [glossary](https://github.com/hcross/ff4/blob/main/docs/primer/00-glossary.md)):
a small program that runs the candidate C and the original interpreted
assembly from an identical entry state and compares the results. A
passing spike is what earns a routine level **L2** on the maturity ladder.

## `desktop/` — proving the whole game still agrees

Individual spikes can't see cross-routine interactions. `desktop/` builds
a full game session (linking the live `ff4-gnw` tree) and runs the
**A/B oracle**: two instances of the game, one with native-C dispatch
enabled and one running purely interpreted, compared frame by frame.
Explained in detail in
[`docs/workflow/validation-oracle.md`](workflow/validation-oracle.md).

## `docs/adr/` — why things are built this way

Architecture Decision Records for the choices that shape everything
above: why native C at all ([ADR-001](adr/adr-001-native-c-port.md)), why
LakeSnes unmodified ([ADR-002](adr/adr-002-lakesnes-upstream.md)), the
translate/delegate split ([ADR-003](adr/adr-003-classification.md)), the
automated prompt-improvement loop
([ADR-004](adr/adr-004-prompt-mutation-loop.md)), and the desktop
validation gate ([ADR 0001](adr/0001-desktop-validation-in-re-loop.md)).

## What is *not* here

Nothing in this repository compiles into the Game & Watch firmware.
Once a candidate in `port/` passes its spike (and, for anything touching
shared state broadly, the whole-game oracle), it is **copied** into
`ff4-gnw/<domain>/` — see
[`ff4-gnw`'s own docs](https://github.com/hcross/ff4-gnw/tree/main/docs)
for what happens to it from there.
