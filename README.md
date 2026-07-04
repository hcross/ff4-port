# ff4-port — FF4 translation workshop

The **production workshop** for the native-C port of Final Fantasy IV (Super
Famicom). Translates 65816 assembly to C, validates every function against the
original ROM, and delivers validated translations to
[hcross/ff4-gnw](https://github.com/hcross/ff4-gnw) for integration into the
Game & Watch firmware.

> **New to reverse engineering, 65816 assembly, or SNES hardware?** Read
> the umbrella repo's primer first —
> [`ff4/docs/primer/`](https://github.com/hcross/ff4/tree/main/docs/primer)
> covers just enough background to follow the rest of this README, with
> pointers to authoritative external references for going deeper. For a
> guided tour of this specific repository's layout and pipeline, see
> [`docs/architecture-overview.md`](docs/architecture-overview.md),
> [`docs/workflow/translation-cascade.md`](docs/workflow/translation-cascade.md)
> and [`docs/workflow/validation-oracle.md`](docs/workflow/validation-oracle.md).

## Role in the ecosystem

```
ff4-port (this repo)                    ff4-gnw (delivery)
  upstream/      65816 disassembly
  translator/    LLM pipeline        →→  battle/, field/, …  (validated C)
  desktop/       wram_diff A/B oracle →→  dispatch_all.c      (updated)
  port/          candidates in flight
  parity/        spike harness
```

**This repo** is the atelier: it holds the source material, the tooling that
generates translation candidates, and the oracle that validates them before
they ship.

**[ff4-gnw](https://github.com/hcross/ff4-gnw)** is the delivery end: only
validated, reviewed functions land there. The `port/` directory in this repo
holds **candidates in flight** — generated drafts awaiting oracle sign-off.
Nothing in `port/` compiles into the firmware.

## Status

**213 routines validated and dispatched** in ff4-gnw
(battle 92, field 114, menu 8, cutscene 20, sound 6).

FF4 boots through the Square Enix splash to the title screen on real
Game & Watch hardware (Mario, STM32H7B0VBT6, 64 MB extflash mod). See
[ff4-gnw](https://github.com/hcross/ff4-gnw) for device-side status and build
instructions.

37 additional candidates are staged in `port/` (25 battle, 12 field) pending
oracle validation.

## Repository layout

```
ff4-port/
├── upstream/            — submodule: everything8215/ff4 disassembly (65816 CA65)
├── LakeSnes/            — submodule: elzo-d/LakeSnes (reference emulator)
├── vanilla/             — your ROM lives here (gitignored)
│
├── desktop/             — A/B validation oracle (the gate before ff4-gnw)
│   ├── wram_diff.c          two-pass harness: dispatch-ON vs pure-interpreter,
│   │                        compare final WRAM; prints divergences
│   ├── oracle_ab.c          frame-level A/B runner
│   ├── miss_profiler.c      dispatch miss profiler
│   └── Makefile
│
├── translator/          — LLM translation pipeline
│   ├── cascade_translate.py     3-stage adaptive cascade: gemma4 → gpt-oss critic → deepseek-v4-pro
│   ├── hardcore_translate.py    single-model multi-turn translator
│   ├── volume_iterate.py        autonomous batch runner over a candidates list
│   ├── port_validated.py        end-to-end: pick PASSes → copy to ff4-gnw → regen dispatch → flash → report
│   ├── prompt_mutation_loop.py  P3 prompt-mutation outer loop (adopts only under no-regression)
│   ├── regression_suite.py      16-routine regression suite used by the mutation loop
│   └── runs/                    JSONL audit trail — every run appended here
│
├── prompts/             — LLM system prompts (versioned)
│   ├── pitfalls.yaml            single source of truth for the 17 documented pitfalls
│   ├── generate_pitfalls.py     renders pitfalls.yaml into both prompts below (--check/--write)
│   ├── reverser_system.md       v2 — pitfalls section generated from pitfalls.yaml
│   ├── reverser_hardcore.md     extends v2 with H1-H5 hardcore sections, same pitfalls
│   └── history/v0/, v1/, v2/    adopted versions with regression scores at adoption
│
├── parity/              — spike harness (function-level parity tests)
│   └── src/spike_*.c            one file per validated spike scenario
│
├── port/                — translation candidates in flight (not yet in ff4-gnw)
│   ├── battle/              25 pending candidates
│   └── field/               12 pending candidates
│
├── ca65-bridge/         — Python: CA65 parser + ADR-003 translate/delegate classifier
└── docs/adr/            — Architecture Decision Records
```

## Validation flow

Every function follows this gate before landing in ff4-gnw:

```
1. ca65-bridge classify    — translate or delegate? (ADR-003)
2. translator/             — LLM generates C candidate → port/<mod>/<fn>.c
3. desktop/wram_diff       — A/B oracle: dispatch-ON vs interpreter, compare WRAM
4. manual review           — cycle budget, flag side-effects, CONTRACT block
5. port_validated.py       — copy to ff4-gnw, regen dispatch_all, commit both repos
```

The `wram_diff` oracle is the hard gate: a function that diverges WRAM does not
ship. Divergences are diagnosed by adjusting cycle injection (`inject_cycles`
for long routines that cross HDMA windows) or by flagging the function as
delegate.

## Quick start

### Prerequisites

- macOS or Linux
- `cc65` — `brew install cc65`
- `node` + `npm` (upstream asset extractor)
- `clang`
- `python` ≥ 3.10
- A legal FF4 JP ROM (`CRC32 CAA15E97`) in `vanilla/`

### Clone

```bash
git clone --recursive https://github.com/hcross/ff4-port.git
cd ff4-port
```

### Build the desktop oracle

```bash
cd desktop && make wram_diff && cd ..
```

### Run the A/B oracle on a savestate

```bash
# Two-pass: dispatch-ON pass A vs pure-interpreter pass B, compare WRAM
desktop/wram_diff <rom.sfc> <savestate.lss>
# Expected: "IDENTICAL" for all validated routines; any divergence is a bug.
```

### Run the batch translator

```bash
# Dry-run (no API call)
python translator/batch_translate.py --module battle --max-functions 5 --dry-run

# Real run via Claude Code subscription
python translator/batch_translate.py --module battle --max-functions 3

# Autonomous volume sweep
python3 translator/volume_iterate.py \
    --names-file /tmp/candidates.txt \
    --chunk-size 5 --max-chunks 50 \
    --max-turns 2 --enable-critic
```

See [`translator/README.md`](translator/README.md) for Anthropic SDK and
OpenAI-compatible (Ollama, OpenRouter) usage.

### Classify a routine

```bash
cd ca65-bridge
.venv/bin/ca65-bridge --root ../upstream classify CalcHits
.venv/bin/ca65-bridge --root ../upstream classify-module battle
```

## Architecture decisions

- [ADR-001](docs/adr/adr-001-native-c-port.md) — Native C port over full emulator port
- [ADR-002](docs/adr/adr-002-lakesnes-upstream.md) — Use LakeSnes upstream unmodified
- [ADR-003](docs/adr/adr-003-classification.md) — Binary classification: translate vs delegate
- [ADR-004](docs/adr/adr-004-prompt-mutation-loop.md) — Automated, regression-gated prompt-mutation loop (P3; dormant, see the ADR's "Current status")
- [ADR 0001](docs/adr/0001-desktop-validation-in-re-loop.md) — Desktop LakeSnes host as the mandatory RE validation gate (separate numbering track — see that file's own header)

## The 17 pitfalls

Single source of truth: [`prompts/pitfalls.yaml`](prompts/pitfalls.yaml),
rendered into `reverser_system.md` and `reverser_hardcore.md` by
`prompts/generate_pitfalls.py` (`--check` for drift, `--write` to
regenerate — see the script's docstring for why `reverser_system.md`
needs care around `prompt_mutation_loop.py`). Highlights:

1. `CMP/BCS` inversion (`bcs` branches when ≥, C uses `<`)
2. Z/N flags must be simulated on entry when jumping past the caller's `LDA`
3. 8-bit arithmetic truncation (`asl` truncates; C `<<` doesn't)
4. Implicit `mf=true` heritage for routines without `shorta`/`longa`
5. Hidden register B preserved across 8-bit `lda` (contaminates `tax → stx`)
6. Long routines crossing HDMA windows need `inject_cycles` not `snes_runCycles`
7. `LDA abs,X` with `xf=0` costs 38 MC (idle unconditionally), not 32 MC
8. When pitfalls 4–5 combine, the function is non-translatable in isolation → delegate

## Translation scope estimate (ADR-003)

| Module    | Routines | Translate (%) | Delegate (%) |
|-----------|---------:|--------------:|-------------:|
| battle    |      373 |           75% |          25% |
| btlgfx    |     1095 |           79% |          21% |
| menu      |      613 |           81% |          19% |
| field     |      651 |           70% |          30% |
| sound     |       16 |           81% |          19% |
| cutscene  |       73 |           82% |          18% |
| **total** | **2821** |       **77%** |      **23%** |

Estimated LLM budget for a full pass (Sonnet + cache): **$3–15**.

## Roadmap

- ✅ Phase 1 — Toolchain bring-up + byte-identical ROM rebuild
- ✅ Phase 2 — Parity harness (double-instance LakeSnes comparator)
- ✅ Phase 3 — ca65-bridge + prompt templates
- ✅ Phase 3.5 — Spike journey (M1–M6) + documented pitfalls
- ✅ Phase 4.0–4.2 — Classifier + multi-provider batch translator
- ✅ Phase 4.3 — Auto-spike generator for parity validation
- ✅ Phase 4.4 — First `battle/` batch run with auto-spike validation
- ✅ Phase 4.5 — P3 prompt-mutation loop (v0 → v1 → v2)
- ✅ Phase 4.6 — Hardcore deepseek-v4-pro tier for the hard tail
- ✅ Phase 4.7 — Multi-turn iterative refinement
- ✅ Phase 4.8 — 3-stage adaptive cascade (gemma4 → critic → deepseek)
- ✅ Phase 5 — Integration into ff4-gnw + game-and-watch-retro-go-sd; boots to title on real hardware
- ⏳ Phase 5.1 — Lift dispatch hit rate beyond 26 % via `volume_iterate.py` sweeps
- ⏳ Phase 5.2 — `RunEmulatedFunc` on G&W so `*_emu` delegates run the original asm at runtime
- ⏳ Phase 5.3 — Savestate loading harness for in-battle dispatch hit-rate measurement

## Upstream

- 65816 disassembly: [everything8215/ff4](https://github.com/everything8215/ff4)
- SNES emulator: [elzo-d/LakeSnes](https://github.com/elzo-d/LakeSnes)
- Delivery overlay: [hcross/ff4-gnw](https://github.com/hcross/ff4-gnw)
- Firmware: [sylverb/game-and-watch-retro-go-sd](https://github.com/sylverb/game-and-watch-retro-go-sd)

## License

MIT. See [LICENSE](LICENSE). The ROM is not licensed and not distributed.
