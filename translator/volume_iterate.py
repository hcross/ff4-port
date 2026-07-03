#!/usr/bin/env python3
"""volume_iterate — autonomous batch runner over a candidates list.

For each chunk of routines:
  1. run cascade_translate (stage-1 gemma4 multi-turn by default; the
     full 3-stage cascade is the cascade_translate.py default if you
     pass --enable-critic)
  2. cp every PASS / delegate_pass into ff4-gnw/<mod>/
  3. regen dispatch_all (auto-stubs *_emu missing)
  4. quick build check in retro-go-sd (make -j8 — no flash, no monitor)
  5. if build OK -> commit + push ff4-gnw, bump retro-go-sd, push
                    and continue with the next chunk
     if build KO -> identify and skip the routine(s) that broke the
                    build (best-effort by parsing the gcc error log),
                    git checkout the affected files, retry from the
                    same chunk minus the offending routine(s).

A `--max-chunks` cap and a `--start-line` offset let the operator
resume after a previous run.

Usage:
    python translator/volume_iterate.py \\
        --names-file /tmp/volume_candidates.txt \\
        --chunk-size 5 \\
        --max-chunks 6 \\
        --start-line 0
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import time
from pathlib import Path

THIS = Path(__file__).resolve().parent
FF4_PORT = THIS.parent
# ff4-gnw is a sibling of ff4-port under the umbrella repo since the
# submodule move (BACKLOG.md §1). Env vars override for a non-standard layout.
FF4_GNW = Path(os.environ.get("FF4_GNW_DIR", str(FF4_PORT.parent / "ff4-gnw")))
RETRO_SD = Path(os.environ.get(
    "RETRO_SD_DIR",
    str(Path.home() / "devel/perso/retrogaming/game-and-watch/game-and-watch-retro-go-sd"),
))

CASCADE = THIS / "cascade_translate.py"
HARDCORE_LOG = THIS / "runs" / "cascade_log.jsonl"

PASS = {"pass", "delegate_pass"}

KEY_PATH = Path.home() / ".ollama" / "ff4-port.api.key"


# ─────────────────────────────────────────────────────────────────────
# Helpers
# ─────────────────────────────────────────────────────────────────────

def run(cmd, cwd=None, timeout=None, check=False, env=None):
    res = subprocess.run(cmd, cwd=str(cwd) if cwd else None,
                         capture_output=True, text=True, timeout=timeout,
                         env=env)
    if check and res.returncode != 0:
        sys.stderr.write(f"[volume] cmd failed (rc={res.returncode}): {cmd}\n")
        sys.stderr.write(f"[volume] stderr_tail: {res.stderr[-800:]}\n")
        raise SystemExit(res.returncode)
    return res


def read_candidates(path: Path, start: int, chunk: int) -> list[tuple[str, str]]:
    lines = [l.strip() for l in path.read_text().splitlines()
             if l.strip() and not l.startswith("#") and ":" in l]
    out = []
    for raw in lines[start:start + chunk]:
        mod, name = raw.split(":", 1)
        out.append((mod.strip(), name.strip()))
    return out


def cascade_chunk(targets: list[tuple[str, str]], max_turns: int,
                    enable_critic: bool) -> list[dict]:
    """Spawn cascade_translate on a small inline names list. Reads
    cascade_log.jsonl after the run and returns the records for THIS
    chunk only (parsed by matching mod:name)."""
    if not targets:
        return []
    log_before = (HARDCORE_LOG.read_text().splitlines()
                  if HARDCORE_LOG.is_file() else [])
    n_before = len(log_before)
    cmd = [sys.executable, str(CASCADE),
           "--names", *[f"{m}:{n}" for m, n in targets],
           "--max-turns", str(max_turns)]
    if not enable_critic:
        cmd.append("--no-critic")
    sys.stderr.write(f"[volume] cascade on {len(targets)} routines...\n")
    sys.stderr.flush()
    proc = subprocess.run(cmd, cwd=str(FF4_PORT), capture_output=False,
                          text=True, timeout=1800)
    if proc.returncode != 0:
        sys.stderr.write(f"[volume] cascade returncode {proc.returncode}\n")
    # Read newly-appended records.
    log_after = HARDCORE_LOG.read_text().splitlines()
    new = [json.loads(l) for l in log_after[n_before:] if l.strip()]
    return new


def port_passes(records: list[dict]) -> list[dict]:
    """For each pass / delegate_pass record, copy the .c into
    ff4-gnw/<mod>/<name>.c. Returns the list of files written."""
    written = []
    for r in records:
        if r.get("final_status") not in PASS:
            continue
        src = r.get("final_c_path") or r.get("c_path")
        if not src or not Path(src).is_file():
            continue
        dst_dir = FF4_GNW / r["mod"]
        dst_dir.mkdir(parents=True, exist_ok=True)
        dst = dst_dir / f"{r['name']}.c"
        dst.write_text(Path(src).read_text())
        written.append({"mod": r["mod"], "name": r["name"],
                        "status": r["final_status"], "dst": str(dst)})
    return written


def regen_dispatch() -> str:
    res = run(["python3", "gen_dispatch.py"], cwd=FF4_GNW, check=True)
    return res.stdout


# ─────────────────────────────────────────────────────────────────────
# Build check
# ─────────────────────────────────────────────────────────────────────

# Match gcc error lines like:
#   external/ff4/field/Foo.c:42:13: error: ...
ERROR_FILE_RE = re.compile(
    r"external/ff4/(\w+)/(\w+)\.c:\d+:\d+: error:"
)


def quick_build() -> tuple[bool, str, set[tuple[str, str]]]:
    """Build retro-go-sd without flashing. Returns (ok, stderr_tail,
    offending_routines_set)."""
    # Pre-flight: the submodule pointer might still be on origin, but
    # we want to see the local ff4-gnw work-tree. Sync it.
    sub = RETRO_SD / "external/ff4"
    run(["git", "fetch", "origin"], cwd=FF4_GNW)
    # The retro-go-sd submodule is a checkout of the same repo; we cp
    # locally instead of relying on git push/pull during a build check.
    # Simpler: rsync the changed file tree (just the .c + dispatch).
    for mod_dir in ("field", "menu", "battle", "cutscene", "btlgfx", "sound"):
        src_dir = FF4_GNW / mod_dir
        dst_dir = sub / mod_dir
        if not src_dir.is_dir():
            continue
        dst_dir.mkdir(parents=True, exist_ok=True)
        for c in src_dir.glob("*.c"):
            shutil.copy(c, dst_dir / c.name)
    for f in ("dispatch_all.c", "dispatch_all.h", "ff4_helpers.c", "ff4_helpers.h"):
        if (FF4_GNW / f).is_file():
            shutil.copy(FF4_GNW / f, sub / f)
    res = subprocess.run(
        ["make", "build/gw_retro_go.elf", "SD_CARD=0", "FF4_AUTOBOOT=1",
         "EXTFLASH_SIZE_MB=4", "-j8"],
        cwd=str(RETRO_SD), capture_output=True, text=True, timeout=300,
    )
    ok = (res.returncode == 0)
    offenders = set()
    for m in ERROR_FILE_RE.finditer(res.stderr + res.stdout):
        offenders.add((m.group(1), m.group(2)))
    return ok, (res.stderr[-3000:] if not ok else ""), offenders


def check_submodules_pristine() -> tuple[bool, list[str]]:
    """Return (ok, dirty_subs). The volume run requires every non-FF4
    submodule of retro-go-sd to match its committed pointer; otherwise
    a local bump (e.g. someone ran `git submodule update` and pulled a
    fatter gwenesis) silently changes the BSS budget and the build
    starts failing further down the chunk series with a misleading
    "RAM_EMU overflow" that points at the FF4 chunks instead of the
    real culprit."""
    res = run(["git", "submodule", "status"], cwd=RETRO_SD)
    dirty = []
    for line in res.stdout.split("\n"):
        if not line.strip():
            continue
        # Format markers: leading space = clean, '+' = modified,
        # '-' = uninitialised, 'U' = merge conflict.
        if line[0] in ("+", "-", "U"):
            path = line.split()[1] if len(line.split()) > 1 else line
            # external/ff4 IS expected to be modified by this script —
            # ignore it.
            if "external/ff4" in path:
                continue
            dirty.append(line.strip())
    return (len(dirty) == 0, dirty)


# Read the relevant size budgets from the linker script so the warning
# threshold survives an upstream change to RAM_EMU_LENGTH.
def read_ram_emu_budget() -> int:
    """Return RAM_EMU length in bytes by parsing the linker script.
    Returns 0 on parse failure."""
    ld = RETRO_SD / "STM32H7B0VBTx_SDCARD.ld"
    if not ld.is_file():
        return 0
    text = ld.read_text()
    m = re.search(r"__RAM_EMU_LENGTH__\s*=\s*(\d+)\s*K", text)
    if m:
        return int(m.group(1)) * 1024
    return 0


def ff4_overlay_bss_size() -> int | None:
    """After a successful build, ask arm-none-eabi-size for the
    .overlay_ff4_bss section size of the FF4 overlay. Returns None if
    we can't parse it (the safety net stays a strict link check)."""
    elf = RETRO_SD / "build/gw_retro_go.elf"
    if not elf.is_file():
        return None
    # First try arm-none-eabi-objdump -h (section headers).
    res = subprocess.run(
        ["arm-none-eabi-objdump", "-h", str(elf)],
        capture_output=True, text=True, timeout=30,
    )
    if res.returncode != 0:
        return None
    # Look for the .overlay_ff4_bss section; size is hex in column 3.
    for line in res.stdout.split("\n"):
        if ".overlay_ff4_bss" in line and "Idx" not in line:
            parts = line.split()
            if len(parts) >= 3:
                try:
                    return int(parts[2], 16)
                except ValueError:
                    continue
    return None


def bss_margin_warn(min_margin_bytes: int) -> tuple[bool, str]:
    """Return (ok, message). ok=False when we should stop the run
    because we're too close to the RAM_EMU budget."""
    budget = read_ram_emu_budget()
    ff4_bss = ff4_overlay_bss_size()
    if not budget or ff4_bss is None:
        return (True, "bss_margin: could not parse — skipping check")
    margin = budget - ff4_bss
    msg = f"bss_margin: ff4_bss={ff4_bss:,} of RAM_EMU={budget:,} (margin={margin:,} bytes)"
    if margin < min_margin_bytes:
        return (False, msg + f"  -- BELOW {min_margin_bytes:,}-byte safety threshold")
    return (True, msg)


# Markers we look for in the FF4_AUTOBOOT monitor stream.
FF4_LIVE_RE = re.compile(
    r"FF4 live: host=(\d+) snes_frame=(\d+) dispatch=(\d+)/(\d+) wall_ms=(\d+)"
)


def hw_verify() -> dict:
    """Flash the device, monitor 35s, parse the autoboot indicators
    and the last FF4 live line. Returns a dict with boot_ok / crash /
    hit_count / total_count / hit_rate / probe_error / notes."""
    # Build artefacts are already up to date from quick_build; just
    # invoke the flash recipe (it depends on build artefacts).
    res = subprocess.run(
        ["make", "flash", "SD_CARD=0", "FF4_AUTOBOOT=1",
         "EXTFLASH_SIZE_MB=4", "-j8"],
        cwd=str(RETRO_SD), capture_output=True, text=True, timeout=600,
    )
    if res.returncode != 0:
        tail = (res.stderr + "\n" + res.stdout)[-2000:]
        # Probe-not-found is benign for the audit (operator's device
        # is just off); we surface it but DON'T treat it as a crash.
        if "Unable to autodetect" in tail or "debugging probe" in tail:
            return {"ok": False, "probe_error": True,
                    "notes": "debug probe unreachable (device off?)",
                    "boot_ok": None, "crash": None}
        return {"ok": False, "probe_error": False,
                "notes": f"flash failed: {tail[-600:]}",
                "boot_ok": False, "crash": True}
    # Capture 35 seconds of serial.
    try:
        mon = subprocess.run(
            ["timeout", "35", "gnwmanager", "monitor"],
            capture_output=True, text=True, cwd=str(RETRO_SD), timeout=50,
        )
    except subprocess.TimeoutExpired:
        return {"ok": False, "probe_error": False,
                "notes": "monitor timed out",
                "boot_ok": False, "crash": True}
    out = mon.stdout
    boot_ok = "FF4_AUTOBOOT_ATTEMPT" in out or "FF4_BOOT_MARKER" in out
    crash = ("Hardfault" in out or "FATAL" in out
             or "boot_magic=0xbad" in out)
    ppu_unblanked = ("forceBlank=0" in out and "nmiEn=1" in out)
    last = None
    for m in FF4_LIVE_RE.finditer(out):
        last = m
    if not last:
        return {"ok": True, "boot_ok": boot_ok, "crash": crash,
                "ppu_unblanked": ppu_unblanked,
                "hit_count": 0, "total_count": 0, "hit_rate": 0.0,
                "probe_error": False,
                "notes": "no FF4 live line in 35s window"}
    host, snes_f, hits, total, wall = map(int, last.groups())
    return {
        "ok": True, "boot_ok": boot_ok, "crash": crash,
        "ppu_unblanked": ppu_unblanked,
        "hit_count": hits, "total_count": total,
        "hit_rate": hits / total if total else 0.0,
        "wall_ms": wall, "host_frame": host,
        "probe_error": False,
        "notes": "",
    }


def revert_routines(routines: list[dict]) -> int:
    """For each routine in the list, `git checkout HEAD` its .c in
    ff4-gnw (effectively un-staging the changes)."""
    n = 0
    for r in routines:
        f = FF4_GNW / r["mod"] / f"{r['name']}.c"
        if not f.is_file():
            continue
        res = run(["git", "checkout", "HEAD", "--", str(f.relative_to(FF4_GNW))],
                  cwd=FF4_GNW)
        if res.returncode == 0:
            n += 1
        else:
            # File was new — git rm it instead.
            f.unlink(missing_ok=True)
            n += 1
    return n


def commit_push_chunk(chunk_n: int, routines: list[dict]) -> str | None:
    if not routines:
        return None
    run(["git", "add", "-A"], cwd=FF4_GNW)
    names = ", ".join(f"{r['mod']}:{r['name']}" for r in routines)
    msg = f"feat(volume): chunk {chunk_n} — {len(routines)} routines"
    body = f"\n\nAdded by volume_iterate.py:\n  {names}\n"
    run(["git", "commit", "-m", msg + body], cwd=FF4_GNW, check=True)
    run(["git", "push", "origin", "main"], cwd=FF4_GNW, check=True)
    res = run(["git", "rev-parse", "--short", "HEAD"], cwd=FF4_GNW)
    sha = res.stdout.strip()

    # Bump submodule in retro-go-sd. First discard the locally-copied
    # build-check files (they were only there to let make resolve the
    # symbols; the authoritative state is on origin/main now).
    sub = RETRO_SD / "external/ff4"
    run(["git", "reset", "--hard", "HEAD"], cwd=sub)
    run(["git", "clean", "-fd"], cwd=sub)
    run(["git", "pull", "origin", "main"], cwd=sub, check=True)
    run(["git", "add", "external/ff4"], cwd=RETRO_SD)
    run(["git", "commit", "-m",
         f"chore(ff4): bump external/ff4 -> {sha} (volume chunk {chunk_n})"],
        cwd=RETRO_SD)
    run(["git", "push", "hcross", "feat/ff4-port-scaffold"],
        cwd=RETRO_SD, check=True)
    return sha


# ─────────────────────────────────────────────────────────────────────
# Main loop
# ─────────────────────────────────────────────────────────────────────

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--names-file", required=True, type=Path)
    ap.add_argument("--chunk-size", type=int, default=5)
    ap.add_argument("--max-chunks", type=int, default=4)
    ap.add_argument("--start-line", type=int, default=0)
    ap.add_argument("--max-turns", type=int, default=2)
    ap.add_argument("--enable-critic", action="store_true", default=False,
                    help="enable stage-2 critic in cascade")
    ap.add_argument("--report-out", type=Path,
                    default=FF4_PORT / "translator/runs/volume_report.json")
    ap.add_argument("--hw-verify-every", type=int, default=5,
                    help="flash device + monitor 35 s every N chunks (default 5; 0 = disabled)")
    ap.add_argument("--bss-warn-margin", type=int, default=5000,
                    help="stop the run when the FF4 overlay BSS gets within "
                         "this many bytes of the RAM_EMU budget (default 5 KB)")
    ap.add_argument("--skip-submodule-check", action="store_true",
                    help="skip the pre-flight check that warns about "
                         "non-FF4 submodules being modified locally")
    args = ap.parse_args()

    if not KEY_PATH.exists():
        sys.exit("no api key at ~/.ollama/ff4-port.api.key")

    sys.stderr.write(f"[volume] candidates from {args.names_file} start={args.start_line} "
                     f"chunk_size={args.chunk_size} max_chunks={args.max_chunks} "
                     f"hw_verify_every={args.hw_verify_every}\n")
    sys.stderr.flush()

    # Patch 1 — submodule pre-flight check.
    if not args.skip_submodule_check:
        clean, dirty = check_submodules_pristine()
        if not clean:
            sys.stderr.write(
                "[volume] ABORT: non-FF4 submodules are modified locally — "
                "this silently changes the RAM_EMU BSS budget and any "
                "build failure later will misattribute the blame to the "
                "wrong commit. Run `git submodule update --recursive` in "
                "retro-go-sd before retrying, or pass --skip-submodule-check.\n"
            )
            for d in dirty:
                sys.stderr.write(f"  dirty: {d}\n")
            sys.exit(2)
        sys.stderr.write("[volume] submodule pre-flight: clean\n")

    report = {
        "start_line": args.start_line,
        "chunks": [],
        "totals": {"attempted": 0, "passed": 0, "ported": 0,
                    "rejected_by_build": 0, "commits": []},
    }

    line = args.start_line
    for c in range(args.max_chunks):
        targets = read_candidates(args.names_file, line, args.chunk_size)
        if not targets:
            sys.stderr.write(f"[volume] candidates exhausted at line {line}\n")
            break
        chunk_id = f"c{c+1}_line{line}"
        sys.stderr.write(f"\n[volume] === chunk {c+1}/{args.max_chunks} "
                         f"(line {line}..{line+len(targets)-1}) ===\n")
        sys.stderr.flush()

        t0 = time.time()
        records = cascade_chunk(targets, args.max_turns, args.enable_critic)
        cascade_dt = time.time() - t0

        written = port_passes(records)
        gen_out = regen_dispatch()
        sys.stderr.write(f"[volume] cascade {cascade_dt:.0f}s, "
                         f"{len(written)} ports, gen_dispatch ok\n")

        ok, err_tail, offenders = quick_build()
        if ok:
            # Patch 2 — BSS margin check.
            bss_ok, bss_msg = bss_margin_warn(args.bss_warn_margin)
            sys.stderr.write(f"[volume] {bss_msg}\n")
            if not bss_ok:
                sys.stderr.write(
                    "[volume] STOP: BSS margin below safety threshold. "
                    "Reverting this chunk's files and stopping the run.\n"
                )
                revert_routines(written)
                regen_dispatch()
                report["chunks"].append({
                    "chunk_id": chunk_id,
                    "targets_count": len(targets),
                    "passes_count": len(written),
                    "build": "ok_bss_too_tight",
                    "bss_message": bss_msg,
                })
                break
            sha = commit_push_chunk(c + 1, written)
            chunk_record = {
                "chunk_id": chunk_id,
                "targets_count": len(targets),
                "passes_count": len(written),
                "build": "ok",
                "ff4_gnw_sha": sha,
                "cascade_sec": round(cascade_dt, 1),
                "bss_message": bss_msg,
            }
            # Patch 3 — periodic hardware verification.
            if (args.hw_verify_every > 0
                    and (c + 1) % args.hw_verify_every == 0):
                sys.stderr.write(f"[volume] hw_verify on chunk {c+1}...\n")
                hv = hw_verify()
                chunk_record["hw_verify"] = hv
                if hv.get("probe_error"):
                    sys.stderr.write(
                        f"[volume] hw_verify: probe unreachable, "
                        f"skipping (notes: {hv['notes']})\n"
                    )
                elif hv.get("crash") or hv.get("boot_ok") is False:
                    sys.stderr.write(
                        f"[volume] STOP: hw_verify detected crash or "
                        f"boot failure at chunk {c+1}. "
                        f"Reverting THIS chunk only and stopping the "
                        f"run for human review.\n"
                    )
                    # Revert ONLY this chunk's commit (the previous
                    # chunks were verified clean on this device or are
                    # untouched). Use `git reset --hard HEAD~1` then
                    # force-push.
                    run(["git", "reset", "--hard", "HEAD~1"], cwd=FF4_GNW)
                    run(["git", "push", "--force-with-lease",
                         "origin", "main"], cwd=FF4_GNW)
                    # Roll the submodule pointer in retro-go-sd back too.
                    sub = RETRO_SD / "external/ff4"
                    run(["git", "reset", "--hard", "HEAD"], cwd=sub)
                    run(["git", "clean", "-fd"], cwd=sub)
                    run(["git", "pull", "origin", "main"], cwd=sub)
                    run(["git", "add", "external/ff4"], cwd=RETRO_SD)
                    run(["git", "commit", "-m",
                         f"chore(ff4): revert chunk {c+1} after hw_verify "
                         f"detected crash"],
                        cwd=RETRO_SD)
                    run(["git", "push", "hcross",
                         "feat/ff4-port-scaffold"], cwd=RETRO_SD)
                    report["chunks"].append(chunk_record)
                    report["totals"]["passed"] += len(written)
                    report["totals"]["rejected_by_build"] += len(written)
                    break
                else:
                    sys.stderr.write(
                        f"[volume] hw_verify OK: dispatch={hv['hit_count']}"
                        f"/{hv['total_count']} "
                        f"(hit_rate={hv['hit_rate']:.3f})\n"
                    )
            report["chunks"].append(chunk_record)
            report["totals"]["passed"] += len(written)
            report["totals"]["ported"] += len(written)
            if sha:
                report["totals"]["commits"].append(sha)
        else:
            # Try to identify and skip the offending routines, then retry
            # by removing them from the chunk.
            sys.stderr.write(f"[volume] BUILD FAILED — offenders: {offenders}\n")
            sys.stderr.write(f"[volume] error_tail: {err_tail[-1500:]}\n")
            offending_records = [w for w in written
                                 if (w["mod"], w["name"]) in offenders]
            kept = [w for w in written if (w["mod"], w["name"]) not in offenders]
            n_reverted = revert_routines(offending_records)
            regen_dispatch()

            # Retry the build with kept routines.
            ok2, err2, off2 = quick_build()
            if ok2 and kept:
                sha = commit_push_chunk(c + 1, kept)
                report["chunks"].append({
                    "chunk_id": chunk_id,
                    "targets_count": len(targets),
                    "passes_count": len(written),
                    "ported_after_revert": len(kept),
                    "reverted": [f"{r['mod']}:{r['name']}" for r in offending_records],
                    "build": "ok_after_revert",
                    "ff4_gnw_sha": sha,
                })
                report["totals"]["passed"] += len(written)
                report["totals"]["ported"] += len(kept)
                report["totals"]["rejected_by_build"] += n_reverted
                if sha:
                    report["totals"]["commits"].append(sha)
            else:
                # Hard failure — revert everything in this chunk and stop.
                revert_routines(written)
                regen_dispatch()
                report["chunks"].append({
                    "chunk_id": chunk_id,
                    "targets_count": len(targets),
                    "passes_count": len(written),
                    "build": "hard_fail",
                    "err_tail": err_tail[-500:],
                })
                sys.stderr.write(f"[volume] HARD FAIL on chunk {c+1}, stopping\n")
                break

        report["totals"]["attempted"] += len(targets)
        line += args.chunk_size

    args.report_out.parent.mkdir(parents=True, exist_ok=True)
    args.report_out.write_text(json.dumps(report, indent=2))
    sys.stderr.write(f"\n[volume] DONE.\n")
    sys.stderr.write(f"[volume] attempted={report['totals']['attempted']} "
                     f"passed={report['totals']['passed']} "
                     f"ported={report['totals']['ported']} "
                     f"rejected={report['totals']['rejected_by_build']} "
                     f"commits={len(report['totals']['commits'])}\n")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
