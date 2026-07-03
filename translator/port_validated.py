#!/usr/bin/env python3
"""port_validated — end-to-end port wrapper for hardcore PASSes.

One command to take the freshly-translated PASS routines from the most
recent hardcore_translate run, copy them into ff4-gnw, regenerate the
dispatch, commit + push both repos, bump the submodule in retro-go-sd,
build + flash the G&W, monitor for 35 s, and report the dispatch
hit-rate delta vs the previous measurement.

Default flow (dry-run by default — pass --commit to actually push):

  1. Read translator/runs/hardcore_log.jsonl, pick the latest run by
     monotonically-increasing `i` field resets (a new run starts at i=1
     after a previous final i=N). Filter status in {pass, delegate_pass}.
  2. For each PASS, cp port/_hardcore/<mod>/<name>.c into
     ff4-gnw/<mod>/<name>.c. Idempotent (auto-include already present
     courtesy of hardcore_translate's post-processor).
  3. Run gen_dispatch.py inside ff4-gnw. Capture stdout.
  4. Commit + push ff4-gnw (if --commit). Otherwise show what would be
     pushed.
  5. cd retro-go-sd/external/ff4 && git pull origin main. Bump the
     submodule pointer in retro-go-sd, commit + push (if --commit).
  6. (--measure) make flash + gnwmanager monitor 35 s + extract the
     last "FF4 live: host=... dispatch=H/T" line. Compare to the
     baseline JSON if --baseline.
  7. Write a JSON report to --report-out.

Usage:

  # Dry-run on the latest hardcore PASSes:
  python translator/port_validated.py

  # Real: push + measure on device:
  python translator/port_validated.py --commit --measure \\
      --baseline /tmp/hit_baseline.json \\
      --report-out /tmp/port_report.json
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
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

DEFAULT_HARDCORE_LOG = FF4_PORT / "translator/runs/hardcore_log.jsonl"
DEFAULT_PASS_STATUSES = {"pass", "delegate_pass"}


# ─────────────────────────────────────────────────────────────────────
# Helpers
# ─────────────────────────────────────────────────────────────────────

def run(cmd, cwd=None, check=False, capture=True, timeout=None):
    """Wrap subprocess.run with sensible defaults."""
    res = subprocess.run(cmd, cwd=str(cwd) if cwd else None,
                         capture_output=capture, text=True, timeout=timeout)
    if check and res.returncode != 0:
        sys.stderr.write(f"[port] cmd failed (rc={res.returncode}): {cmd}\n")
        sys.stderr.write(f"[port] stderr: {res.stderr[-1500:]}\n")
        raise SystemExit(res.returncode)
    return res


def read_latest_run(log_path: Path,
                     pass_statuses: set[str]) -> list[dict]:
    """Return the list of PASS records from the LATEST run in the JSONL.
    A new run is detected by `i` dropping to 1 (or the file ending)."""
    if not log_path.is_file():
        return []
    runs: list[list[dict]] = [[]]
    for line in log_path.read_text().splitlines():
        if not line.strip():
            continue
        try:
            rec = json.loads(line)
        except json.JSONDecodeError:
            continue
        if rec.get("i") == 1 and runs[-1]:
            runs.append([])
        runs[-1].append(rec)
    latest = runs[-1]
    out = []
    for rec in latest:
        s = rec.get("final_status") or rec.get("status")
        if s in pass_statuses and rec.get("c_path"):
            out.append({
                "mod":  rec["mod"],
                "name": rec["name"],
                "c_path": rec["c_path"],
                "status": s,
            })
    return out


# ─────────────────────────────────────────────────────────────────────
# Step 1-3 : port files + regen dispatch
# ─────────────────────────────────────────────────────────────────────

def cp_into_ffgnw(pass_records: list[dict]) -> list[dict]:
    """Copy each PASS .c into ff4-gnw/<mod>/<name>.c. Returns the list
    of files actually written (skips unchanged content)."""
    written = []
    for r in pass_records:
        src = Path(r["c_path"])
        if not src.is_file():
            sys.stderr.write(f"[port] WARN missing src: {src}\n")
            continue
        dst_dir = FF4_GNW / r["mod"]
        dst_dir.mkdir(parents=True, exist_ok=True)
        dst = dst_dir / f"{r['name']}.c"
        new = src.read_text()
        if dst.is_file() and dst.read_text() == new:
            continue
        dst.write_text(new)
        written.append({"mod": r["mod"], "name": r["name"], "dst": str(dst)})
    return written


def regen_dispatch() -> str:
    res = run(["python3", "gen_dispatch.py"], cwd=FF4_GNW, check=True)
    return res.stdout


# ─────────────────────────────────────────────────────────────────────
# Step 4-5 : commit + push both repos
# ─────────────────────────────────────────────────────────────────────

def git_has_changes(repo: Path) -> bool:
    res = run(["git", "status", "--porcelain"], cwd=repo)
    return bool(res.stdout.strip())


def commit_push_ff4_gnw(commit: bool, message: str) -> str | None:
    if not git_has_changes(FF4_GNW):
        return None
    run(["git", "add", "-A"], cwd=FF4_GNW, check=True)
    if not commit:
        res = run(["git", "diff", "--cached", "--stat"], cwd=FF4_GNW)
        sys.stderr.write(f"[port] ff4-gnw (dry-run) staged:\n{res.stdout}\n")
        return None
    run(["git", "commit", "-m", message], cwd=FF4_GNW, check=True)
    run(["git", "push", "origin", "main"], cwd=FF4_GNW, check=True)
    res = run(["git", "rev-parse", "--short", "HEAD"], cwd=FF4_GNW)
    return res.stdout.strip()


def bump_submodule_and_push(commit: bool, message: str) -> str | None:
    sub = RETRO_SD / "external/ff4"
    run(["git", "pull", "origin", "main"], cwd=sub, check=True)
    if not git_has_changes(RETRO_SD):
        return None
    run(["git", "add", "external/ff4"], cwd=RETRO_SD, check=True)
    if not commit:
        res = run(["git", "diff", "--cached", "--stat"], cwd=RETRO_SD)
        sys.stderr.write(f"[port] retro-go-sd (dry-run) staged:\n{res.stdout}\n")
        return None
    run(["git", "commit", "-m", message], cwd=RETRO_SD, check=True)
    run(["git", "push", "hcross", "feat/ff4-port-scaffold"], cwd=RETRO_SD,
         check=True)
    res = run(["git", "rev-parse", "--short", "HEAD"], cwd=RETRO_SD)
    return res.stdout.strip()


# ─────────────────────────────────────────────────────────────────────
# Step 6 : build, flash, monitor, measure
# ─────────────────────────────────────────────────────────────────────

FF4_LIVE_RE = re.compile(
    r"FF4 live: host=(\d+) snes_frame=(\d+) dispatch=(\d+)/(\d+) "
    r"wall_ms=(\d+)"
)


def build_and_flash() -> dict:
    sys.stderr.write("[port] make flash ...\n")
    res = run(
        ["make", "flash", "SD_CARD=0", "FF4_AUTOBOOT=1",
         "EXTFLASH_SIZE_MB=4", "-j8"],
        cwd=RETRO_SD, timeout=600,
    )
    if res.returncode != 0:
        return {"ok": False, "rc": res.returncode,
                "stderr_tail": res.stderr[-2000:]}
    return {"ok": True}


def monitor_and_extract(seconds: int = 35) -> dict:
    sys.stderr.write(f"[port] monitor {seconds}s ...\n")
    try:
        res = subprocess.run(
            ["timeout", str(seconds), "gnwmanager", "monitor"],
            capture_output=True, text=True, cwd=str(RETRO_SD), timeout=seconds + 15,
        )
    except subprocess.TimeoutExpired:
        return {"ok": False, "error": "monitor timed out"}
    out = res.stdout
    # Find the LAST FF4 live line
    last_live = None
    for m in FF4_LIVE_RE.finditer(out):
        last_live = m
    if not last_live:
        return {"ok": False, "error": "no FF4 live line in 35s",
                "tail": out[-1500:]}
    host, snes_f, hits, total, wall = map(int, last_live.groups())
    return {
        "ok": True, "host": host, "snes_frame": snes_f,
        "hits": hits, "total": total,
        "hit_rate": hits / total if total else 0.0,
        "wall_ms": wall,
        # also surface boot indicators
        "boot_ok": "FF4_AUTOBOOT_ATTEMPT" in out,
        "ppu_unblanked": ("forceBlank=0" in out
                          and "nmiEn=1"     in out),
        "crash":      ("Hardfault" in out or "FATAL" in out),
    }


def compare_to_baseline(now: dict, baseline_path: Path | None) -> dict:
    if not baseline_path or not baseline_path.is_file():
        return {"baseline": None, "delta": None}
    base = json.loads(baseline_path.read_text())
    if not base.get("ok"):
        return {"baseline": base, "delta": None}
    delta_pp = (now["hit_rate"] - base["hit_rate"]) * 100
    return {
        "baseline": {k: base[k] for k in ("hits", "total", "hit_rate")},
        "delta_pp": round(delta_pp, 2),
        "regression": delta_pp < -1.0,
    }


# ─────────────────────────────────────────────────────────────────────
# CLI
# ─────────────────────────────────────────────────────────────────────

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--hardcore-log", type=Path, default=DEFAULT_HARDCORE_LOG)
    ap.add_argument("--commit", action="store_true",
                    help="actually commit + push both repos "
                         "(default: dry-run, just stage and show stat)")
    ap.add_argument("--measure", action="store_true",
                    help="after the port, make flash + monitor 35s + "
                         "report the hit rate")
    ap.add_argument("--baseline", type=Path, default=None,
                    help="JSON from a previous --measure run, used to "
                         "compute delta_pp")
    ap.add_argument("--report-out", type=Path,
                    default=FF4_PORT / "translator/runs/port_report.json")
    ap.add_argument("--message",
                    default="feat(ff4): port hardcore PASS routines (autonomous)",
                    help="git commit subject line")
    ap.add_argument("--statuses", nargs="+",
                    default=list(DEFAULT_PASS_STATUSES),
                    help="which final_status values to consider PASS")
    ap.add_argument("--names", nargs="+", default=None,
                    help="bypass hardcore_log and port these mod:name "
                         "entries (each must already have a .c under "
                         "port/_hardcore/<mod>/<name>.c)")
    args = ap.parse_args()

    # ----- Step 1 : gather PASS records -----
    if args.names:
        passes = []
        for s in args.names:
            mod, name = s.split(":", 1)
            cpath = FF4_PORT / "port" / "_hardcore" / mod / f"{name}.c"
            passes.append({"mod": mod, "name": name,
                           "c_path": str(cpath), "status": "pass"})
    else:
        passes = read_latest_run(args.hardcore_log, set(args.statuses))
    sys.stderr.write(f"[port] {len(passes)} PASS routines to port\n")
    for p in passes:
        sys.stderr.write(f"  {p['mod']}:{p['name']}\n")

    if not passes:
        sys.exit("nothing to port")

    # ----- Step 2 : copy into ff4-gnw -----
    written = cp_into_ffgnw(passes)
    sys.stderr.write(f"[port] wrote {len(written)} files into ff4-gnw\n")

    # ----- Step 3 : regen dispatch + auto-stubs -----
    regen_out = regen_dispatch()
    sys.stderr.write(f"[port] gen_dispatch.py output:\n{regen_out}\n")

    # ----- Step 4 : commit + push ff4-gnw -----
    gnw_sha = commit_push_ff4_gnw(args.commit, args.message)
    sys.stderr.write(f"[port] ff4-gnw HEAD: {gnw_sha}\n")

    # ----- Step 5 : bump retro-go-sd submodule + push -----
    sd_sha = None
    if args.commit:
        sd_sha = bump_submodule_and_push(args.commit,
                                          f"chore(ff4): bump external/ff4 "
                                          f"(auto-port via port_validated)")
        sys.stderr.write(f"[port] retro-go-sd HEAD: {sd_sha}\n")

    # ----- Step 6 : measure on device -----
    measurement = None
    delta = None
    if args.measure:
        bf = build_and_flash()
        if not bf["ok"]:
            measurement = {"ok": False, "stage": "build_flash", **bf}
        else:
            measurement = monitor_and_extract(seconds=35)
            if measurement["ok"]:
                delta = compare_to_baseline(measurement, args.baseline)
                # Update the baseline if --baseline path looks like a
                # rolling pointer (rename: optional).

    # ----- Step 7 : report -----
    report = {
        "ported": len(written),
        "routines": [{"mod": p["mod"], "name": p["name"]} for p in passes],
        "ff4_gnw_head": gnw_sha,
        "retro_go_sd_head": sd_sha,
        "measurement": measurement,
        "delta": delta,
        "commit_mode": "real" if args.commit else "dry-run",
    }
    args.report_out.parent.mkdir(parents=True, exist_ok=True)
    args.report_out.write_text(json.dumps(report, indent=2))
    sys.stderr.write(f"[port] report written to {args.report_out}\n")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
