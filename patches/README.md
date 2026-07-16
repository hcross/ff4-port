# patches/ — translation-patch variants of the FF4 ROM

Support for community translation patches, per the umbrella repo's
translation-patch ADR: **one language = one canonical pre-patched ROM image**,
built offline here, identified everywhere by its CRC32. At `ff4_init` the
runtime CRCs the loaded ROM and applies the matching *dispatch profile*
(vanilla → all native routines; known variant → the routines whose original
asm the patch rewrote are gated back to the interpreter, which reads the
patched bytes and is therefore always correct).

There is deliberately **no runtime `--patch` option** in the desktop binaries
and no on-device patching (the device ROM lives in read-only XIP flash): a
single byte path per variant means a single CRC32 on desktop and device.

## Layout

| Path | Tracked | Purpose |
|------|---------|---------|
| `apply_ips.py` | yes | IPS applier: header-mode handling, power-of-2 padding, modified-range report |
| `manifest.json` | yes | Known patches: sources, hashes, application parameters, validation status |
| `spike_check.py` | yes | Re-runs a routine's proven spike against a variant image (registry/patch_impact.py's not-gated set); the per-routine empirical backstop for what static analysis cannot see |
| `spike_checks.json` | yes | GENERATED sidecar: per-(dispatch ID, variant CRC32) spike-check verdicts |
| `tests/` | yes | pytest suite for the applier |
| `files/` | **no** | patch payloads (`.ips`), user-supplied — hashes pinned in the manifest |
| `out/` | **no** | built variant images + `.report.json` range reports |

## Usage

```sh
# Reproduce a known variant (verifies patch sha256, base CRC32, output hashes):
python3 patches/apply_ips.py --patch-id j2e-en-v321

# Explicit mode, e.g. while qualifying a new patch:
python3 patches/apply_ips.py BASE.sfc PATCH.ips -o out/variant.sfc \
    --headered auto --pad auto --expect-base-crc32 CAA15E97 --report out/variant.report.json
```

The `.report.json` lists every modified ROM range (unheadered file offsets +
SNES PCs, half-open `end_excl`); it is the input of
`registry/patch_impact.py`, which intersects those ranges with per-routine
spans (`registry/dispatch_ranges.json`) to derive the variant's dispatch
profile.

## Variant validation seeds (`out/seeds/`, untracked)

Savestates used by the oracle runs recorded in the manifest. Vanilla fixtures
cannot load against a 2 MiB variant (the statehandler's `romSize` guard), so
each variant gets its own seeds, built headlessly from reset with `--press`
scripts and `--save`. The J2e set and its regeneration recipe:

| Seed | How |
|------|-----|
| `j2e-001-arrival-dialogue.lss` | reset → `--frames 12000 --press start:950` + `--press a:F` every 200 frames from 1100 |
| `j2e-002-cain-dialogue.lss` | chain one more 12000-frame A-mash run from 001 (`--load` → `--save`) |
| `j2e-003-baron-freeroam.lss` | three more chained 12000-frame A-mash runs (free roam in Baron castle) |
| `j2e-004-menu-open.lss` | from 003: `--frames 600 --press x:100` |

Promotion of variant seeds into the private `fixtures/` submodule (catalogued
in FIXTURES.md) is the maintainer's call.

**Durable coverage sidecar**: `ff4-desktop-oracle`'s `--json` output always
includes `native_hits` — every dispatch PC the seed's run actually exercised
natively. When (re-)validating a seed, point `--json` at
`out/seeds/<seed>.oracle.json` (a sidecar next to the `.lss`, not `/tmp`) so
it becomes a durable, greppable index of "which seed reaches which routine"
— workflows/WF-VALID.md step 1 consults these for its opportunistic variant
pass.

## Adding a new patch

1. Drop the `.ips` under `files/`, add a manifest entry (source URL, sha256,
   base identity, header mode — `--headered auto` usually detects it; the
   classic signature is `max_end % 0x8000 == 0x200`). The base's
   `rom_identities` entry must carry `"lineage": "jp"` (see below) for the
   patch to be eligible for anything past step 3.
2. Build with `--patch-id`, smoke-boot the image in the desktop harness
   (`ff4-desktop-headless out/<img>.sfc --frames 900 --press start:950 …`).
3. Run the impact analysis and regenerate the dispatch profiles
   (`registry/patch_impact.py`), review the disabled set (also written to
   `registry/VARIANT_GAPS.md`).
4. Empirically confirm the not-gated set: `python3 patches/spike_check.py
   --sweep --variant <id>` (cheap, per-routine, reuses each proven vanilla
   spike — see spike_check.py's docstring for why it always boots on
   vanilla and only swaps the ROM buffer after the baseline is captured).
   Any `diverged` verdict is real signal a modified/callee range was
   missed; fix via `registry/extra_ranges.json` before proceeding.
5. Oracle A/B on the patched image per the validation workflow, then record
   `validation.status` (`smoke-boot` → `desktop-validated` → `device-validated`)
   with evidence. Every rung is proven reachable: `j2e-en-v321` walked the
   full ladder to `device-validated` (2026-07-15 bench — see its manifest
   entry for what each rung's evidence looks like).

Base-ROM lineage matters: only patches targeting the **JP ROM** the dispatch
table was proven against can run natively — `rom_identities[*].lineage` in
the manifest gates this (`spike_check.py` skips any non-`"jp"` base
outright, since `TARGET_ADDR_24` in every generated spike is resolved
against the JP disassembly). US-lineage patches (e.g. Namingway Edition,
base *Final Fantasy II* US v1.1) have foreign code layouts — on desktop
they can run interpreter-only (`--no-dispatch`); on device they are refused
by the ROM-identity guard.

Registry discipline: `registry/dispatch_state.jsonl` L-levels stay scoped to
the **vanilla** ROM; per-variant validity lives in this manifest only.
