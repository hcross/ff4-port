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
FF4_GNW = Path.home() / "devel/perso/retrogaming/ff4-gnw"
RETRO_SD = Path.home() / "devel/perso/retrogaming/game-and-watch/game-and-watch-retro-go-sd"

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
    args = ap.parse_args()

    if not KEY_PATH.exists():
        sys.exit("no api key at ~/.ollama/ff4-port.api.key")

    sys.stderr.write(f"[volume] candidates from {args.names_file} start={args.start_line} "
                     f"chunk_size={args.chunk_size} max_chunks={args.max_chunks}\n")
    sys.stderr.flush()

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
            sha = commit_push_chunk(c + 1, written)
            report["chunks"].append({
                "chunk_id": chunk_id,
                "targets_count": len(targets),
                "passes_count": len(written),
                "build": "ok",
                "ff4_gnw_sha": sha,
                "cascade_sec": round(cascade_dt, 1),
            })
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
