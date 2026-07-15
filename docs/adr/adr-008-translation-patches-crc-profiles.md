# ADR-008 — Translation patches via canonical variant images + CRC-keyed dispatch profiles

- **Status**: Accepted 2026-07-15
- **Date**: 2026-07-15
- **Deciders**: Hoani Cross (scope decision), claude-code
- **Scope**: language/translation support across all three repos —
  `ff4-port/patches/` (applier + manifest), umbrella `registry/`
  (ranges + impact analysis), `ff4-gnw` (rom_ident + dispatch gate),
  retro-go-sd scaffold (Language option + refusal screen). Filed here
  because this repo is the project's ADR home (see ADR-007).

## Context

The port should play in languages other than Japanese, using community
translation patches (user request named Namingway Edition; J2e is the
other major English patch). Two hard facts shape the design:

1. **The dispatch table is an equivalence proof against ONE image.**
   ~200 native C routines intercept JSR/JSL at fixed PCs and were proven
   equivalent against FF4 JP rev 1.1 (CRC32 `CAA15E97`) only. A patch
   that rewrites the original asm of a dispatched routine silently
   invalidates its native body.
2. **Patch lineages differ.** Namingway Edition (and Project II it
   builds on) target the **US** ROM (*Final Fantasy II* US v1.1) — a
   foreign code layout where none of our PCs mean anything. J2e v3.21
   targets the **JP** ROM (headered IPS, expands 1 MiB → 1.5 MiB,
   rewrites the 16×8 font engine). The text engine itself (decode
   `$00:B459`, draw `$01:82CD`, DTE `$13:9700`) is NOT dispatched — it
   runs interpreted and reads live ROM, so translated text is honored
   automatically.

Device constraints: the ROM is a borrowed pointer into read-only XIP
extflash (85 KB MCU heap — no runtime patching), and `snes_loadRom`
requires power-of-2 sizes. Desktop and device must run the exact same
bytes for a CRC to mean anything.

## Decision

**One language = one canonical pre-patched ROM image + one generated
dispatch profile keyed by the image's full-file CRC32.**

- `ff4-port/patches/apply_ips.py` builds the canonical image offline
  (base-CRC validation, headered-IPS handling, pad to power-of-2) and
  emits a modified-range report. `manifest.json` pins every hash and
  the per-variant validation status. No runtime `--patch` option
  anywhere; the image file is passed like any ROM.
- `registry/gen_ranges.py` derives per-routine spans from a **proven**
  out-of-band `ca65 -g` + `ld65 --dbgfile` relink (byte-identical to
  the true ROM modulo the header checksum). Unresolvable entries (code
  living in regions the disassembly models as data — banks 12/15/16
  et al.) are marked `resolved:false`.
- `registry/patch_impact.py` byte-diffs vanilla vs variant, intersects
  with routine spans (± slop), honours frozen-data dependencies
  (`extra_ranges.json`), propagates through the transitive asm callee
  closure (a native body may inline a callee), **exempts DELEG**
  wrappers (they run the patched asm via `run_emulated_func`), and
  **always-gates unresolved entries** (fail-closed). Output:
  `ff4-gnw/rom_profiles.c` (generated, `--check`-guarded) + a
  human-review report with a size alarm.
- `ff4-gnw/rom_ident.c` CRCs the image once at `ff4_init` and arms a
  **per-slot gate array** in `dispatch_all.c`: gated hooks fall through
  to the interpreter (correct on patched bytes by construction), are
  counted separately from misses, and cost vanilla one always-false
  byte test per hit. Unknown CRC: the device (`FF4_REQUIRE_KNOWN_ROM`)
  refuses with an on-screen message; desktop warns and runs
  interpreter-only.
- **The oracle on the patched image is the final judge**: A = native +
  profile, B = pure interpreter, on variant-specific seeds (vanilla
  fixtures refuse to load cross-size by the statehandler `romSize`
  guard). Static analysis proposes, the oracle disposes.
- Device UX v1 (zelda3 model): a persisted **Language** pause-menu
  option selects the ROM file at the next launch. Registry L-levels
  stay vanilla-scoped; per-variant validity lives in the manifest.

## Consequences

- J2e EN v3.21 (CRC `F135CAE6`) ships as the first variant: 67 gated
  PCs (13 real overlap/callee hits + 54 fail-closed unresolved). The
  >25 alarm was reviewed with a dispatch-hit probe: none of the 54
  ever fires on boot/title/intro and the per-frame hot set is not
  gated; residual watch = `UpdateMode7Regs`/`UpdateWipe*` (world map /
  transitions) — measure D6 on device, resolve ranges manually to
  un-gate if cadence suffers. Oracle: IDENTICAL on 4 seeds; regress
  baselines per-CRC under `.regress-baselines/<CRC32>/`.
- Namingway Edition stays **out of device scope** (US lineage → zero
  applicable dispatches → interpreter-only → below the device cadence
  floor, ADR-007). It remains loadable on desktop for study. A future
  US-lineage requalification would slot into the same profile
  mechanism unchanged.
- Adding a language = drop the IPS + one manifest entry + one
  `patch_impact.py` run + oracle seeds. No registry schema change.
- Saves/savestates are per-variant (name encodings differ); documented,
  not engineered around.

## Rejected alternatives

- **Reusing `ff4_dispatch_filter` for profiles** — the oracle installs
  its own filter (`--exclude`/`--only`); the profile would vanish
  exactly during validation runs. Hence the separate gate array.
- **Partial-file hashing** (first+last 64 KiB) — invents a second
  identity, saves <100 ms once at boot, aliases padded variants.
- **Runtime IPS application on device** — ROM is read-only XIP; a
  patched copy cannot fit the 85 KB heap; sparse overlays are invisible
  to the `s_readPtr` fast path and to direct `cart->rom[...]` reads in
  ported C.
- **Annotation-based ranges** (monolithic `notes/ff4j-sfc.asm`) — the
  +2 drift class and missing banks; replaced by the proven ld65 relink.
- **Deferred, not rejected**: source-level translation builds (the
  upstream disassembly already carries `LANG_SUFFIX`/`ff4-en` infra) —
  would produce expansion-free variants with segment-exact diffs, and
  the CRC-keyed profiles consume any image source unchanged.

## Evidence

- `ff4-port/patches/manifest.json` (hashes, validation status, seeds)
- `ff4-port/patches/out/j2e-en-v321.impact.json` (gated set + reasons)
- Oracle runs 2026-07-15: IDENTICAL ×4 J2e seeds; `gated=0` vanilla /
  `gated=22` J2e boot; vanilla regress verdicts unchanged.
- MemPalace: wing=ff4-gnw, room=architecture-decisions ("J2e v3.21
  canonical application facts"), room=task-handoff
  (`ff4-translation-patches`).
