#!/usr/bin/env python3
"""spike_check — re-run a routine's proven spike against a translation-patch
variant image, to catch drift the static byte-range analysis
(registry/patch_impact.py) cannot see: a frozen ROM-data copy baked into the
ported C, or an indirect jump target the transitive callee-closure walk
missed.

Why this is meaningful (and when it is NOT): a spike proves C is equivalent
to run_emulated_func-interpreted asm at an identical, PRNG-fuzzed entry
state (translator/generate_spike.py). Re-running that same proof against a
variant's canonical ROM image, at the same target PC, is a valid extra
confirmation ONLY for a dispatch entry patch_impact.py judged NOT gated for
that variant (its byte-diff does not overlap this routine's own range or
its transitive callees' ranges). An entry patch_impact.py already gated is
EXPECTED to diverge under the variant's asm -- that IS the point of gating,
the interpreter (not this C body) runs there in production -- so gated
entries are silently skipped here, never reported as failures.

Base-ROM lineage matters: TARGET_ADDR_24 in every generated spike is
resolved against the JP disassembly, so address preservation for an
untouched routine only holds for a JP-lineage base. A manifest patch whose
base resolves to a non-'jp' lineage is skipped outright (see
manifest.json's rom_identities[*].lineage and its comment).

Results persist in patches/spike_checks.json (GENERATED, tracked) -- never
in registry/dispatch_state.jsonl, whose L-levels stay scoped to the vanilla
ROM (ADR-008 / patches/README.md convention); this is the variant-facing
sidecar next to manifest.json, keyed by (dispatch_id, variant CRC32).

Every check runs a VANILLA CONTROL pass first, same trial count, before the
variant pass. Some routines fail to re-spike at all through the naive
filename convention this tool uses (e.g. the documented off-by-2 class
where ca65-bridge's address resolution disagrees with REVERSED_FUNCTION,
WF-DECOMP.md; or a routine promoted via the custom-spike-author path,
bundled in a shared file this tool's find_source() cannot locate) -- that
is a PRE-EXISTING tool/resolution limitation, unrelated to any variant, and
reported as "control_failed" / "no_source", never as "diverged". A
"diverged" verdict is reserved for: control passes under vanilla, but the
SAME spike fails under the variant image -- that is the real signal.

The variant pass itself ALWAYS boots and snapshots its entry-state baseline
from the VANILLA rom, then swaps ONLY the cart's ROM buffer to the variant
image before running trials (generate_spike.py's --variant-rom). Booting
directly on the variant image was tried first and rejected: a variant's
own (possibly patched) early boot code leaves undeclared CPU/WRAM context
(e.g. the carry flag, or any WRAM byte the routine reads but the CONTRACT
doesn't fuzz) in a different state than vanilla's boot does, which then
masqueraded as "diverged" for routines with no real dependency on the
patch at all -- 37 of 122 false positives measured 2026-07-16, including
plain register-only math helpers (Mult8_c) that cannot plausibly be
affected by any ROM-content change. Swapping post-baseline isolates the
comparison to exactly the code bytes at/after the target PC, which is what
this tool is actually meant to prove.

Usage:
    python3 patches/spike_check.py D<id> --variant <patch-id> [--trials N]
    python3 patches/spike_check.py D<id> --all-variants [--trials N]
    python3 patches/spike_check.py --sweep [--variant <patch-id>] [--trials N]

A DIVERGED result on a not-gated entry is a genuine, rare signal: the
static analysis missed something. Remedy printed inline: add the missing
data dependency to registry/extra_ranges.json and rerun
registry/patch_impact.py so the gate catches it -- never silently patch the
gate from here.

Exit code: 0 if every attempted check passed (or none were eligible); 1 if
any diverged; 2 on usage error.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

HERE = Path(__file__).resolve().parent            # ff4-port/patches
PORT_ROOT = HERE.parent                           # ff4-port
UMBRELLA = PORT_ROOT.parent                        # ff4/
REGISTRY_DIR = UMBRELLA / "registry"
FFGNW = UMBRELLA / "ff4-gnw"
GEN_SPIKE = PORT_ROOT / "translator" / "generate_spike.py"
MANIFEST = HERE / "manifest.json"
CHECKS_OUT = HERE / "spike_checks.json"

MODULE_DIRS = ["battle", "field", "menu", "cutscene", "sound", "btlgfx"]
SPIKE_SUFFIX = "variant"  # dedicated namespace; never assumes another tool
                          # (qualify.py's "qualify" suffix) already built one
_SUMMARY_RE = re.compile(r"trials:\s*(\d+),\s*fails:\s*(\d+)")


# --------------------------------------------------------------------------
# Lookups (mirrors translator/qualify.py's find_source exactly -- same
# ff4-gnw/<module>/<name>.c convention, the promoted-C final location)
# --------------------------------------------------------------------------
def find_source(base_name: str) -> Path | None:
    for mod in MODULE_DIRS:
        cand = FFGNW / mod / f"{base_name}.c"
        if cand.is_file():
            return cand
    return None


def load_registry() -> dict[str, dict]:
    state = REGISTRY_DIR / "dispatch_state.jsonl"
    out: dict[str, dict] = {}
    with state.open() as fh:
        for line in fh:
            line = line.strip()
            if line:
                rec = json.loads(line)
                out[rec["id"]] = rec
    return out


def load_manifest() -> dict:
    return json.loads(MANIFEST.read_text())


def eligible_variants(manifest: dict) -> list[dict]:
    """Known patches whose base is JP-lineage (address-preserving)."""
    out = []
    for p in manifest["patches"]:
        ident = manifest["rom_identities"].get(p["base"], {})
        if ident.get("lineage") == "jp":
            out.append(p)
    return out


def gated_pcs_for(patch: dict) -> set[str]:
    """PCs registry/patch_impact.py gated for this variant (its impact.json).
    Missing file -> treat as "nothing known gated" (caller still needs the
    variant image to exist to actually run anything)."""
    impact_path = HERE / "out" / f"{patch['id']}.impact.json"
    if not impact_path.is_file():
        return set()
    data = json.loads(impact_path.read_text())
    return {g["pc"] for g in data.get("gated", [])}


def variant_image_path(patch: dict) -> Path:
    return HERE / "out" / patch["output"]["file"]


VANILLA_ROM = PORT_ROOT / "upstream" / "rom" / "ff4-jp1.sfc"


# --------------------------------------------------------------------------
# Sidecar (patches/spike_checks.json)
# --------------------------------------------------------------------------
def load_checks() -> dict:
    if CHECKS_OUT.is_file():
        return json.loads(CHECKS_OUT.read_text())
    return {
        "comment": "GENERATED by patches/spike_check.py -- per-(dispatch_id, "
                   "variant CRC32) spike-vs-variant verdicts. Never fed into "
                   "registry/dispatch_state.jsonl (vanilla-scoped L-levels, "
                   "ADR-008); this is the variant-facing sidecar next to "
                   "manifest.json. A 'diverged' status is real signal: it means "
                   "patch_impact.py's static gating missed a data dependency "
                   "for that routine under that variant -- see registry/"
                   "extra_ranges.json and rerun patch_impact.py.",
        "checks": [],
    }


def upsert_check(doc: dict, result: dict) -> None:
    checks = doc["checks"]
    for i, c in enumerate(checks):
        if c["dispatch_id"] == result["dispatch_id"] and c["variant_crc32"] == result["variant_crc32"]:
            checks[i] = result
            return
    checks.append(result)


def save_checks(doc: dict) -> None:
    doc["checks"].sort(key=lambda c: (c["dispatch_id"], c["variant_crc32"]))
    CHECKS_OUT.write_text(json.dumps(doc, indent=2) + "\n")


# --------------------------------------------------------------------------
# Core check
# --------------------------------------------------------------------------
def _run_spike(src: Path, trials: int, variant_rom: Path | None) -> tuple[str | None, int, int, str]:
    """Invoke generate_spike.py --build --run, always booting on VANILLA
    (see swap_variant_rom's docstring in generate_spike.py for why: the
    variant image, if given, is only swapped in AFTER the baseline is
    captured, via --variant-rom -- never used as the boot rom itself).
    Returns (error_status_or_None, trials_ran, fails, detail)."""
    cmd = [sys.executable, str(GEN_SPIKE), str(src), "--build", "--run", str(trials),
          "--rom", str(VANILLA_ROM), "--spike-suffix", SPIKE_SUFFIX]
    if variant_rom is not None:
        cmd += ["--variant-rom", str(variant_rom)]
    try:
        res = subprocess.run(
            cmd, capture_output=True, text=True, timeout=120, cwd=str(PORT_ROOT),
        )
    except subprocess.TimeoutExpired:
        return "timeout", 0, 0, ""
    out = (res.stdout or "") + "\n" + (res.stderr or "")
    if "CONTRACT_MMIO_MISMATCH" in out or "CUSTOM_SPIKE" in out:
        return "build_error", 0, 0, (out.strip().splitlines()[-1] if out.strip() else "")
    m = _SUMMARY_RE.search(out)
    if not m:
        return "build_error", 0, 0, f"no trial summary in output (tail: {out.strip().splitlines()[-3:]!r})"
    return None, int(m.group(1)), int(m.group(2)), ""


def run_one(dispatch_id: str, rec: dict, patch: dict, trials: int) -> dict:
    base = rec["name"][:-2] if rec["name"].endswith("_c") else rec["name"]
    result = {
        "dispatch_id": dispatch_id, "name": rec["name"], "variant": patch["id"],
        "variant_crc32": patch["output"]["crc32"],
        "timestamp": datetime.now(timezone.utc).isoformat(timespec="seconds"),
        "status": None, "trials": trials, "fails": None, "detail": None,
    }
    src = find_source(base)
    if src is None:
        result["status"] = "no_source"
        return result
    if "// CONTRACT:" not in src.read_text():
        result["status"] = "no_contract"
        return result
    image = variant_image_path(patch)
    if not image.is_file():
        result["status"] = "no_variant_image"
        result["detail"] = (f"{image} missing -- build it first: "
                            f"python3 patches/apply_ips.py --patch-id {patch['id']}")
        return result

    # Vanilla control FIRST (no variant swap): some routines fail to
    # re-spike through this tool's naive filename convention regardless of
    # any variant (the documented off-by-2 ca65-bridge class, or a
    # custom-spike-bundled routine) -- attribute that to the tool, not to
    # the variant.
    err, _, ctrl_fails, detail = _run_spike(src, trials, variant_rom=None)
    if err:
        result["status"] = "control_failed"
        result["detail"] = f"vanilla control run itself errored ({err}): {detail}"
        return result
    if ctrl_fails > 0:
        result["status"] = "control_failed"
        result["detail"] = (f"vanilla control diverged too ({ctrl_fails}/{trials} fails) -- "
                            f"pre-existing tool/resolution limitation for this routine, "
                            f"not a {patch['id']}-specific finding; see WF-DECOMP.md's "
                            f"off-by-2 / custom-spike notes")
        return result

    # Variant pass: same baseline (booted on vanilla above), ROM swapped
    # in post-baseline -- isolates the comparison to the variant's code
    # bytes without inheriting its own boot sequence as a confound.
    err, trials_ran, fails, detail = _run_spike(src, trials, variant_rom=image)
    if err:
        result["status"] = "build_error"
        result["detail"] = detail
        return result
    result["trials"], result["fails"] = trials_ran, fails
    result["status"] = "pass" if fails == 0 else "diverged"
    return result


def check_one_id(dispatch_id: str, registry: dict, patches: list[dict], trials: int) -> list[dict]:
    rec = registry.get(dispatch_id)
    if rec is None:
        print(f"error: no dispatch with id {dispatch_id!r}", file=sys.stderr)
        return []
    if rec["level"] == "DELEG":
        # Wrappers execute the original asm via run_emulated_func -- they
        # already honor the patched bytes; nothing to spike-check.
        return []
    out = []
    for patch in patches:
        if rec["pc"].upper() in gated_pcs_for(patch):
            continue  # gated: expected to diverge, not our concern here
        out.append(run_one(dispatch_id, rec, patch, trials))
    return out


def report(result: dict) -> None:
    tag = result["status"].upper()
    line = f"  {result['dispatch_id']} {result['name']:<28} {result['variant']:<16} {tag}"
    if result["status"] in ("pass", "diverged"):
        line += f" ({result['fails']}/{result['trials']} fails)"
    print(line)
    if result["status"] == "diverged":
        print(f"    !! not gated for {result['variant']} but the spike diverged under its image.")
        print(f"       This means patch_impact.py's static analysis missed a real "
              f"dependency for {result['dispatch_id']}. Remedy: add the missing "
              f"data/callee dependency to registry/extra_ranges.json, then rerun "
              f"registry/patch_impact.py so the gate catches it -- do not hand-edit "
              f"the gate.")
    elif result["status"] == "control_failed":
        print(f"    (not a variant finding) {result['detail']}")
    elif result["detail"]:
        print(f"    {result['detail']}")


# --------------------------------------------------------------------------
def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("dispatch_id", nargs="?", help="e.g. D0081F4 (omit with --sweep)")
    ap.add_argument("--variant", help="check against one manifest patch id")
    ap.add_argument("--all-variants", action="store_true",
                    help="check against every known JP-lineage variant")
    ap.add_argument("--sweep", action="store_true",
                    help="check every L2+ dispatch not gated for the selected variant(s)")
    ap.add_argument("--trials", type=int, default=200)
    args = ap.parse_args(argv)

    if args.sweep == bool(args.dispatch_id):
        ap.error("pass either a dispatch_id or --sweep, not both/neither")
    if not args.sweep and not (args.variant or args.all_variants):
        ap.error("pass --variant ID or --all-variants")

    manifest = load_manifest()
    all_eligible = eligible_variants(manifest)
    if args.variant:
        patches = [p for p in all_eligible if p["id"] == args.variant]
        if not patches:
            known = ", ".join(p["id"] for p in manifest["patches"])
            non_jp = [p["id"] for p in manifest["patches"] if p not in all_eligible]
            hint = f" (non-JP-lineage, excluded: {non_jp})" if args.variant in non_jp else ""
            print(f"error: no JP-lineage variant '{args.variant}' (known: {known}){hint}",
                  file=sys.stderr)
            return 2
    else:
        patches = all_eligible

    registry = load_registry()
    doc = load_checks()
    results: list[dict] = []

    if args.sweep:
        for dispatch_id, rec in registry.items():
            if rec["level"] not in ("L2", "L3", "L4"):
                continue
            results.extend(check_one_id(dispatch_id, registry, patches, args.trials))
    else:
        results = check_one_id(args.dispatch_id, registry, patches, args.trials)

    if not results:
        print("(no eligible (routine, variant) pairs -- gated, DELEG, or no JP-lineage variant known)")
        return 0

    for r in results:
        upsert_check(doc, r)
        report(r)
    save_checks(doc)

    diverged = [r for r in results if r["status"] == "diverged"]
    return 1 if diverged else 0


if __name__ == "__main__":
    sys.exit(main())
