# ADR-003 — Binary classification: translate vs delegate

- **Status**: Accepted
- **Date**: backfilled 2026-07-04 (the decision predates this document —
  see "Note on backfilled ADRs" in ADR-001)
- **Deciders**: Hoani Cross
- **Scope**: `ff4-port` (`ca65-bridge`'s classifier, the `translator/`
  pipeline, the `reverser_system.md`/`reverser_hardcore.md` LLM prompts)

## Context

Not every one of the ~2,821 routines catalogued in `ff4-port/README.md`'s
"Translation scope estimate" is a good candidate for translation to C.
Large, deeply-composed routines are harder to translate correctly (more
surface area for a subtle equivalence bug) and harder to review by eye.
The LLM-assisted translation pipeline (`translator/batch_translate.py`)
also needs a deterministic, machine-checkable answer *before* it prompts
the model, because the prompt itself branches on it (`reverser_system.md`:
`mode: translate` produces a full C body, `mode: delegate` produces a thin
wrapper) — an ad hoc, case-by-case human judgment call would not scale to
batch translation, nor be reproducible or auditable at this volume.

## Decision

Classify every routine, before any translation attempt, into exactly one
of two modes via `ca65-bridge classify` (`ca65_bridge/backend.py`):

- **`translate`** — the default. Produces a complete C function body,
  proven equivalent to the original via the per-routine spike (see
  `docs/workflow/translation-cascade.md`).
- **`delegate`** — a 5-10 line wrapper named `<func>_emu(snes, args)` that
  sets up CPU state (DB, DP, register widths, inputs) and calls
  `run_emulated_func(snes, ADDR)`, letting the interpreter execute the
  original asm at runtime. No C body is written or reviewed for these.

A routine is classified `delegate` when **any** of these measurable
signals holds (`reverser_system.md`, "Pre-check: function classification"):

1. `instr_count > 50` — likely a composition/orchestration function, not
   a single well-scoped unit of logic.
2. `call_count > 2` — multi-delegation; cached register/bank state from
   one callee can silently leak into the next, a hard-to-verify-by-eye
   interaction.
3. Contains an `lda <X> / tax / stx <Y>` chain — triggers Pitfall 9
   (hidden accumulator high byte preserved across an 8-bit `lda`), a
   known source of silent translation bugs.
4. Has `longa` without a final `shorta` — the routine changes the
   accumulator width and never restores it, risking caller mode
   pollution if translated naively.

## Consequences

### Positive
- Deterministic and reproducible: the same routine always gets the same
  classification, independent of which human or LLM session looks at it.
- Directly drives the LLM prompt's branch (`mode: translate` /
  `mode: delegate`), so the classification and the translation attempt
  can't drift apart.
- Concentrates translation effort (and review risk) on routines that are
  actually tractable, instead of spending equal scrutiny on a 5-instruction
  leaf function and a 200-instruction orchestrator.

### Negative
- A `delegate`d routine gets **no** native-C performance benefit (ADR-001's
  whole motivation) — it keeps running interpreted forever unless someone
  manually decides to hand-translate it later.
- The thresholds (`instr_count > 50`, `call_count > 2`, ...) are heuristics
  tuned empirically, not derived from a formal cost model; they can
  misclassify in either direction (see `DISPATCH_REGISTRY.md`'s `DELEG`
  entries for routines where delegation was later reconsidered).

## Alternatives rejected

- **Translate every routine to C regardless of size/complexity.** Rejected:
  the project's own experience (the `custom_spike`/`fixable_l1` buckets in
  `registry/next_task.py`, and the multiple "false L2" incidents recorded
  in `prompts/pitfalls.yaml` — routines whose CONTRACT under-declared their
  real effects) shows translation error rate rises sharply with routine
  size and composition depth. Forcing every routine through the same path
  would concentrate risk precisely where it's hardest to catch.
- **No formal rule — decide per routine by manual judgment.** Rejected as
  unreproducible and unauditable at ~2,821 routines, and incompatible with
  driving an LLM prompt that needs a definite `mode:` before it can even
  start.
- **Delegate everything, translate nothing.** Rejected: it defeats
  ADR-001's entire purpose. If no routine is ever translated, the
  interpreter-only performance ceiling (~4-6 fps, see ADR-001) is never
  broken.
