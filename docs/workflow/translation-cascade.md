# The translation pipeline: from assembly to a C candidate

> Source of truth for the exact steps to run: [`../../../workflows/WF-DECOMP.md`](https://github.com/hcross/ff4/blob/main/workflows/WF-DECOMP.md)
> in the umbrella repo. This page explains *why* the pipeline is shaped
> the way it is, not what commands to type.

## Why an LLM is doing assembly-to-C translation at all

Hand-translating ~2,800 routines (see the "Translation scope estimate"
table in the [README](../../README.md)) one at a time would be extremely
slow. An LLM, given the right context (the assembly, the surrounding
conventions, a list of known pitfalls), can produce a first-draft C
translation far faster than typing it by hand — but "faster" is only
useful if the result can be **mechanically proven correct**, not just
plausible-looking. That's why translation and validation are strictly
separate stages: the LLM proposes, the spike (see
[`validation-oracle.md`](validation-oracle.md) and the umbrella
[primer](https://github.com/hcross/ff4/blob/main/docs/primer/03-reverse-engineering-and-shadow-execution.md))
disposes. Nothing is trusted because an LLM wrote it confidently.

## Step 1 — classify before translating

Before any translation attempt, `ca65-bridge classify` decides
**translate** (full C body) or **delegate** (thin wrapper calling back
into the interpreter) for the routine, per
[ADR-003](../adr/adr-003-classification.md). This decision is fed into
the LLM prompt as `mode: translate` / `mode: delegate` — the LLM doesn't
make this call itself.

## Step 2 — the prompt carries the hard-won lessons

The system prompt (`prompts/reverser_system.md`, and a "hardcore" variant
for the hardest routines, `prompts/reverser_hardcore.md`) is not just
"translate this assembly to C." It carries a growing, single-sourced list
of **known pitfalls** — `prompts/pitfalls.yaml`, rendered into both prompt
files — each one describing a specific, previously-encountered translation
mistake and how to avoid it (the direct-page trap from the umbrella
primer is Pitfall 1; treating a hardware register write as a plain memory
write is Pitfall 13). Every pitfall exists because a real translation got
it wrong once; the list is how that lesson stays learned.

## Step 3 — an adaptive cascade of models

Not every routine needs the most expensive model. `cascade_translate.py`
tries progressively more capable (and more expensive) tiers only as
needed: a fast baseline model first, escalating to a stronger model plus
an automated critique pass, and finally a slower, more thorough
"hardcore" tier (multi-turn, iterative refinement) reserved for routines
that resist the earlier tiers. This keeps the common case cheap and fast
while still having a fallback for the genuinely hard tail of routines.

## Step 4 — the prompt itself can improve automatically

A separate, opt-in mechanism ([ADR-004](../adr/adr-004-prompt-mutation-loop.md))
can take a routine that fails translation, ask a critic LLM for an
improved system prompt, and adopt it **only if** it fixes that routine
without breaking a fixed regression suite of previously-passing ones. This
is how new pitfalls have occasionally been discovered automatically rather
than only by a human noticing a pattern. It is currently dormant — see
the ADR for the operational caution around re-enabling it.

## Step 5 — the output is a CONTRACT, not just code

Every translated function comes with a structured `// CONTRACT:` comment
declaring exactly which registers and RAM addresses it reads and writes,
and which hardware side effects (MMIO/DMA) it triggers. This isn't
documentation for humans only — the spike generator
(`translator/generate_spike.py`) reads it to know exactly what to compare.
An incomplete CONTRACT (one that omits a real hardware side effect) is a
documented, repeat failure mode — see Pitfall 16 in `prompts/pitfalls.yaml`
— which is why the CONTRACT format itself is treated as part of the
correctness surface, not an afterthought.

## What happens next

A translated candidate lands in `port/<module>/<func>.c`. It is not
trusted yet — see [`validation-oracle.md`](validation-oracle.md) for how
it earns that trust.
