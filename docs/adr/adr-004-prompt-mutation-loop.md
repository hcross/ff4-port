# ADR-004 — Automated, regression-gated prompt-mutation loop (P3)

- **Status**: Accepted, implementation dormant since 2026-06-13 (see
  "Current status" below)
- **Date**: backfilled 2026-07-04 (the decision predates this document —
  see "Note on backfilled ADRs" in ADR-001)
- **Deciders**: Hoani Cross, claude-code
- **Scope**: `ff4-port/translator/` (`prompt_mutation_loop.py`,
  `gpt_oss_critic.py`, `regression_suite.py`), `ff4-port/prompts/`
  (`reverser_system.md`, `reverser_task.md`, `reverser_examples.md`)

## Context

`reverser_system.md` (the LLM system prompt driving the `translator/`
pipeline's baseline tier, see `docs/workflow/translation-cascade.md`) needs
to keep improving as new failure patterns are discovered across the
translation backlog — the same handful of failure classes recur
(`WRONG_SIGNATURE`, `COMPILE_ERROR`, `NO_CODE_OR_TRUNCATED`, per the
baseline calibration in `prompts/history/v0/manifest.json`: 23% pass rate,
7/30 on the `gemma4:31b` tier). Hand-editing the prompt every time a new
failure is diagnosed doesn't scale, and risks a human unconsciously
weakening a passage that was fixing a different, older failure.

## Decision

Build an outer loop (`translator/prompt_mutation_loop.py`) that closes this
by machine:

1. Pick a routine that currently **fails** translation under the live
   prompt.
2. Ask a critic LLM (`gpt_oss_critic.py`, gpt-oss-120b) for a **full
   candidate replacement** `reverser_system.md` that would plausibly fix
   this specific failure, given the asm, the generated (broken) C, and the
   error class/message.
3. Re-test the failing routine against the candidate.
4. If it now passes, run the **full regression suite**
   (`regression_suite.py`, a fixed set of previously-passing routines).
5. Adopt the candidate as the new live prompt **only if the regression
   suite shows zero regressions** — otherwise discard it and try again
   (`--max-consecutive-rejects` bounds the retries).

Every adopted version is archived under `prompts/history/vK/` with a
`manifest.json` recording the mutation summary, the routine that drove it,
and the regression score at adoption — an audit trail, not just a diff.

## Consequences

### Positive
- Went from a 23% baseline pass rate (v0) through two adopted mutations
  (v1: "fixed `field:MapGfxBankTbl` (WRONG_SIGNATURE)"; v2: "multi-pass
  critic + examples/task in context") without a human hand-editing prose.
- The **zero-regression gate** is the load-bearing safety property: a
  prompt candidate that fixes one routine by accidentally breaking the
  wording that was keeping another routine correct is rejected
  automatically, not caught later by chance.
- v1 demonstrates the loop can genuinely discover new content, not just
  reword existing text: it added an entirely new pitfall (Pitfall 11 —
  "Goto labels followed by a declaration") that no human had written down
  yet at that point.

### Negative — and the reason this ADR exists as a warning, not just a record
`reverser_system.md` is consequently **not a purely hand-authored file**:
this loop can overwrite it wholesale at any time it is re-run, including
inventing new pitfalls or rewording existing ones on its own initiative.
This has a direct, documented interaction with `prompts/pitfalls.yaml`
(the single-source-of-truth mechanism built in this project on 2026-07-04,
see `prompts/generate_pitfalls.py`'s own docstring): if this loop is ever
re-run and adopts a new version, whoever resumes must run
`generate_pitfalls.py --check` **before** any `--write`, and manually fold
any pitfall the critic added or reworded back into `pitfalls.yaml` first —
otherwise `--write` silently discards the loop's improvement the next time
someone regenerates the prompts from the yaml.

### Current status
Dormant since the v2 adoption (2026-06-13). No CI or cron trigger invokes
it — it only runs when explicitly invoked. Not retired: anyone can re-run
it, which is precisely the scenario the warning above exists for.

## Alternatives rejected

- **Manual-only prompt iteration.** Rejected as the status quo this ADR
  replaces: the same handful of failure classes kept recurring across the
  routine backlog, and manual edits do not carry an automatic regression
  check against the routines that were already passing.
- **Always adopt the critic's proposed prompt, no regression gate.**
  Rejected as unacceptably risky: an LLM-proposed rewrite optimizing for
  one failing routine has no guarantee it preserves the wording that made
  other routines pass; the regression suite is what makes adoption safe
  enough to automate.
- **Fine-tune a dedicated translation model instead of iterating the
  prompt.** Rejected as out of scope for a project at this resource scale
  — prompt engineering against off-the-shelf models is far cheaper than
  curating a fine-tuning dataset and managing a custom model.
