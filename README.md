# ff4-port

Native C reimplementation of **Final Fantasy IV** (Super Famicom),
following the `snesrev/zelda3` shadow-execution pattern. Target platform:
`game-and-watch-retro-go-sd` (STM32H7B0 Game & Watch Mario mod).

## Status

**Experimental — Phase 4 scaffold complete.** Six validated spikes (M1–M6)
prove the shadow-execution architecture works on FF4 with unmodified
LakeSnes upstream. Three functions translated to native C with parity
validation (1000+ trials each), two functions classified as delegated
under [ADR-003](docs/adr-003-classification.md).

## What this project is

- A **batch translator pipeline** that converts 65816 assembly from the
  [everything8215/ff4](https://github.com/everything8215/ff4) disassembly
  to native C, validated function-by-function against the original ROM
  running under [LakeSnes](https://github.com/elzo-d/LakeSnes) emulation.
- A **classifier** (ADR-003) that decides per-function whether to
  translate to C or delegate to emulated execution.
- A **parity harness** that runs both implementations side-by-side and
  diffs WRAM / VRAM / SRAM / OAM / CGRAM frame-by-frame.

## What this project is NOT

- Not a ROM. The original ROM is copyright Square Enix; users provide
  their own legally obtained copy. See [`vanilla/README.md`](vanilla/README.md).
- Not yet playable. The translator pipeline is the focus. Phase 5
  (G&W integration) is documented but not implemented.

## Repository layout

```
ff4-port/
├── vanilla/             — your ROM lives here (gitignored)
├── upstream/            — submodule: everything8215/ff4 disassembly
├── LakeSnes/            — submodule: elzo-d/LakeSnes emulator
├── parity/              — parity harness (C, links LakeSnes core)
│   ├── src/
│   │   ├── parity_compare.c       — generic double-instance comparator
│   │   ├── spike_calchits_m2.c    — M2: CalcHits translation (1000/1000 ✓)
│   │   ├── spike_apply_dmg_mult_m3.c — M3: ApplyDmgMult (1008/1008 ✓)
│   │   ├── spike_get_dmg_ptr_m4.c — M4: GetDmgPtr (256/256 ✓)
│   │   ├── spike_calc_dmg_m5.c    — M5: CalcDmg (delegate proof)
│   │   └── spike_apply_dmg_m6.c   — M6: ApplyDmg (delegate validation)
│   └── Makefile
├── ca65-bridge/         — Python: parses ca65 sources + ADR-003 classifier
│   ├── ca65_bridge/
│   │   ├── parsers/asm.py
│   │   ├── backend.py    — REBackend-compatible interface
│   │   └── cli.py        — `ca65-bridge get-asm <name>`, `classify-module`
│   └── tests/
├── prompts/             — LLM templates for the translator
│   ├── reverser_system.md     — 10 documented pitfalls
│   ├── reverser_task.md
│   └── reverser_examples.md   — CalcHits + ApplyDmgMult few-shots
├── translator/          — batch translator with pluggable LLM provider
│   ├── batch_translate.py
│   ├── llm_providers.py — claude CLI / Anthropic SDK / OpenAI-compat
│   └── README.md
└── port/                — generated native C output (per module)
```

## Quick start

### Prerequisites

- macOS or Linux (developed on macOS arm64)
- `cc65` (for upstream ROM build) — `brew install cc65`
- `node` + `npm` (for upstream's asset extractor)
- `clang` (for parity harness)
- `sdl2` (for LakeSnes desktop, optional) — `brew install sdl2`
- `python` ≥ 3.10
- A legal FF4 ROM in `vanilla/` (see [`vanilla/README.md`](vanilla/README.md))

### Clone with submodules

```bash
git clone --recursive https://github.com/hcross/ff4-port.git
cd ff4-port
```

### Build everything

```bash
# 1. Place your ROM
cp <your-rom>.bin vanilla/
cp vanilla/<your-rom>.bin upstream/vanilla/ff4-jp.sfc

# 2. Build the upstream ROM (validates ROM + builds reference binary)
cd upstream && npm install && make rip && make ff4-jp1 && cd ..

# 3. Build LakeSnes (optional, for parity desktop)
cd LakeSnes && make && cd ..

# 4. Build parity harness
cd parity && make && cd ..

# 5. Set up the ca65-bridge Python package
cd ca65-bridge && uv venv .venv && uv pip install -e ".[dev]" --python .venv/bin/python && cd ..
```

### Run the spikes (parity validation)

```bash
cd parity
./ff4-parity-compare ../upstream/rom/ff4-jp1.sfc ../upstream/rom/ff4-jp1.sfc 6000  # self-consistency
./ff4-spike-calchits-m2 ../upstream/rom/ff4-jp1.sfc 1000                          # M2: CalcHits
./ff4-spike-apply-dmg-mult ../upstream/rom/ff4-jp1.sfc 1000                       # M3
./ff4-spike-get-dmg-ptr ../upstream/rom/ff4-jp1.sfc                               # M4 (exhaustive)
./ff4-spike-calc-dmg ../upstream/rom/ff4-jp1.sfc 200                              # M5 delegate
./ff4-spike-apply-dmg ../upstream/rom/ff4-jp1.sfc                                 # M6 delegate
```

### Classify a routine

```bash
cd ca65-bridge
.venv/bin/ca65-bridge --root ../upstream classify CalcHits
.venv/bin/ca65-bridge --root ../upstream classify-module battle
```

### Run the batch translator (dry-run, no API call)

```bash
python translator/batch_translate.py --module battle --max-functions 5 --dry-run
```

### Run the batch translator (real, claude CLI — uses Claude Code subscription)

```bash
python translator/batch_translate.py --module battle --max-functions 3
```

See [`translator/README.md`](translator/README.md) for Anthropic SDK and
OpenAI-compatible (Ollama, OpenRouter) usage.

## Architecture decisions

- [ADR-001](docs/adr-001-native-c-port.md) — Voie B: native C port over
  full emulator port
- [ADR-002](docs/adr-002-lakesnes-upstream.md) — Use LakeSnes upstream
  unmodified (no fork)
- [ADR-003](docs/adr-003-classification.md) — Binary classification:
  translate vs delegate

## Methodology — the 10 pitfalls

The `prompts/reverser_system.md` template documents 10 pitfalls
discovered during the spike journey. Each was caught and resolved by the
parity harness. Highlights:

1. CMP/BCS inversion (`bcs` branches when ≥, C uses `<`)
2. Z/N flags must be simulated on routine entry when jumping past the
   caller's `LDA`
3. 8-bit arithmetic truncation (`asl` truncates; C `<<` doesn't)
4. Implicit `mf=true` heritage for routines without `shorta`/`longa`
5. Hidden register B preserved across mode A 8-bit `lda` (contaminates
   `tax → stx`)
6. ADR-003 — when 4–5 above combine, function is non-translatable in
   isolation → delegate

Full list and remediation patterns in `prompts/reverser_system.md`.

## Project ratio (ADR-003 estimate)

| Module    | Routines | Translate (%) | Delegate (%) |
|-----------|---------:|--------------:|-------------:|
| battle    |      373 |           75% |          25% |
| btlgfx    |     1095 |           79% |          21% |
| menu      |      613 |           81% |          19% |
| field     |      651 |           70% |          30% |
| sound     |       16 |           81% |          19% |
| cutscene  |       73 |           82% |          18% |
| **total** | **2821** |       **77%** |      **23%** |

Note: "routines" includes internal sub-labels; the practical count of
"business functions" requiring real translation is ~200–400. Estimated
LLM budget for a full pass (Sonnet + cache): **$3–15**.

## Roadmap

- ✅ Phase 1 — Toolchain bring-up + byte-identical ROM rebuild
- ✅ Phase 2 — Parity harness (double-instance LakeSnes comparator)
- ✅ Phase 3 — ca65-bridge + prompt templates
- ✅ Phase 3.5 — Spike journey (M1–M6) + 10 documented pitfalls
- ✅ Phase 4.0–4.2 — Classifier + multi-provider batch translator
- ⏳ Phase 4.3 — Auto-spike generator for parity validation
- ⏳ Phase 4.4 — First end-to-end run on `damage.asm`
- ⏳ Phase 5 — Integration into `game-and-watch-retro-go-sd/external/ff4/`

## License

MIT. See [LICENSE](LICENSE). The ROM is not licensed and not distributed.
