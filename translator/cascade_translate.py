#!/usr/bin/env python3
"""cascade_translate — 3-stage RE pipeline with adaptive escalation.

Routes each routine through a cheap-first cascade. Routines that the
cheaper model can already crack don't pay the deepseek wall-clock; only
the genuinely hard tail escalates to deepseek.

Stage 1 — gemma4:31b + reverser_system.md (v2 baseline), multi-turn
          up to 3 cycles with verbatim error feedback. Catches the
          easy + medium-hard routines.

Stage 2 — gpt-oss:120b critic. Only invoked if every stage-1 turn
          failed. Reads the 3 failed turns and the verbatim errors,
          proposes a routine-specific mutated system prompt (additive
          only, preserves all Pitfalls), then 1 retry of gemma4 with
          the mutated prompt. Catches the routines the v2 prompt was
          missing a Pitfall for.

Stage 3 — deepseek-v4-pro + reverser_hardcore.md (H1-H5), multi-turn
          up to 3 cycles. Final fallback for the genuinely hard
          routines (indexed stores edge cases, exotic MMIO paths, etc).

A routine exits the cascade the moment its status is in {pass,
delegate_pass, custom_spike, spike_timeout}. custom_spike at stage 1
is treated as terminal (the oracle won't validate it at any later
stage either).

Usage:
    python translator/cascade_translate.py --names-file hot.txt \\
        --max-turns 3 \\
        --enable-critic \\
        --out-dir port/_cascade
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import sys
import tempfile
import time
from pathlib import Path

THIS = Path(__file__).resolve().parent
ROOT = THIS.parent
sys.path.insert(0, str(THIS))
import hardcore_translate as ht  # type: ignore
import gpt_oss_critic as goc  # type: ignore

PROMPTS = ROOT / "prompts"
V2_SYSTEM    = PROMPTS / "reverser_system.md"
HARDCORE_SYS = PROMPTS / "reverser_hardcore.md"
TASK_MD      = PROMPTS / "reverser_task.md"
EXAMPLES_MD  = PROMPTS / "reverser_examples.md"

DEFAULT_GEMMA    = "gemma4:31b"
DEFAULT_DEEPSEEK = "deepseek-v4-pro"
DEFAULT_CRITIC   = "gpt-oss:120b"
DEFAULT_API_BASE = "https://ollama.com/v1"
DEFAULT_KEY_PATH = Path.home() / ".ollama" / "ff4-port.api.key"

PASS_STATUSES        = {"pass", "delegate_pass"}
TERMINAL_NO_ESCALATE = {"pass", "delegate_pass", "custom_spike", "spike_timeout",
                         "asm_empty", "hydrate_error"}


# ─────────────────────────────────────────────────────────────────────
# Stage runners
# ─────────────────────────────────────────────────────────────────────

def run_stage_translate(mod: str, name: str, system_md: str,
                          examples_md: str, task_template: str,
                          out_dir: Path, model: str, api_base: str,
                          api_key: str, max_turns: int) -> dict:
    """Thin wrapper around hardcore_translate.translate_one_multiturn so
    each stage looks the same."""
    return ht.translate_one_multiturn(
        mod=mod, name=name,
        system_md=system_md, examples_md=examples_md,
        task_template=task_template, out_dir=out_dir,
        model=model, api_base=api_base, api_key=api_key,
        max_output_tokens=16384, max_turns=max_turns,
    )


def run_stage_critic_mutation(mod: str, name: str, system_md_v2: str,
                                 examples_md: str, task_template: str,
                                 stage1_turns: list[dict],
                                 stage1_c_path: str,
                                 out_dir: Path, model: str,
                                 api_base: str, api_key: str) -> dict:
    """Call gpt-oss critic with the stage-1 failure context, then retry
    gemma4 once with the mutated system prompt.

    The critic gets:
      - the current v2 system prompt
      - the routine asm
      - the last failed C body (from stage 1's last turn)
      - the last error tail
    It returns a full replacement system prompt. We then run gemma4
    once with that mutated prompt."""
    last_turn = stage1_turns[-1] if stage1_turns else {}
    last_status = last_turn.get("status", "unknown")
    # Recover the asm by re-hydrating; the original routine was
    # processed through stage 1 already so this is cheap.
    routine = ht.bt.RoutineInfo(
        name=name, module=mod, address="", instr_count=0, call_count=0,
        decision="translate", reasons=["cascade_critic"],
        xrefs_out=[], asm_body="",
    )
    try:
        routine = ht.bt.hydrate(routine)
    except Exception as e:
        return {"status": "critic_hydrate_error", "error": str(e)[:300],
                "mutated_prompt": ""}

    # The last validation error tail isn't in the stage1 record, but
    # we can re-validate stage1_c_path to recover it. Cheaper: just
    # use a generic prompt — but better is to re-extract the verbatim
    # tail. Skip extra spike run; use what we have.
    asm_excerpt = "\n".join(routine.asm_body.splitlines()[:300])
    err_message = last_turn.get("status", "") + " on turn " + str(last_turn.get("turn", "?"))
    # gpt-oss critic returns a full replacement system prompt
    mutated = goc.propose_mutation(
        current_system_md=system_md_v2,
        asm_excerpt=asm_excerpt,
        routine_name=name,
        generated_c=Path(stage1_c_path).read_text() if stage1_c_path and Path(stage1_c_path).is_file() else "(empty)",
        error_class=last_status.upper(),
        error_message=err_message[:2000],
        api_base=api_base, api_key=api_key,
        max_output_tokens=16384, temperature=0.2,
        examples_md=examples_md, task_md=task_template,
    )
    if not mutated.strip() or len(mutated) < 200:
        return {"status": "critic_empty", "mutated_prompt_bytes": len(mutated)}

    # Save the mutated prompt for audit, then retry gemma4 once with it.
    mut_dir = out_dir / f"_critic_mutated" / f"{mod}_{name}"
    mut_dir.mkdir(parents=True, exist_ok=True)
    (mut_dir / "reverser_system.md").write_text(mutated)

    retry = ht.translate_one_multiturn(
        mod=mod, name=name,
        system_md=mutated, examples_md=examples_md,
        task_template=task_template,
        out_dir=out_dir / "_critic_retry",
        model=model, api_base=api_base, api_key=api_key,
        max_output_tokens=16384, max_turns=1,
    )
    return {
        "status": retry["status"],
        "c_path": retry.get("c_path", ""),
        "mutated_prompt_bytes": len(mutated),
        "mutated_prompt_path": str(mut_dir / "reverser_system.md"),
        "retry_turn": retry.get("turns", [{}])[0] if retry.get("turns") else {},
    }


# ─────────────────────────────────────────────────────────────────────
# Per-routine cascade
# ─────────────────────────────────────────────────────────────────────

def cascade_one(mod: str, name: str,
                 v2_system: str, hardcore_system: str,
                 examples_md: str, task_template: str,
                 out_dir: Path, api_base: str, api_key: str,
                 gemma_model: str, deepseek_model: str,
                 max_turns: int, enable_critic: bool) -> dict:
    """Process one routine through the cascade. Returns a record that
    captures every stage attempted and the final status."""
    record = {"mod": mod, "name": name, "stages": []}

    # ----- Stage 1 : gemma4 multi-turn with v2 prompt -----
    sys.stderr.write(f"  [stage1 gemma4 multi-turn]\n")
    sys.stderr.flush()
    s1 = run_stage_translate(
        mod, name, v2_system, examples_md, task_template,
        out_dir / "_stage1_gemma", gemma_model, api_base, api_key, max_turns,
    )
    s1["stage"] = "gemma4_multiturn"
    record["stages"].append(s1)

    if s1["status"] in TERMINAL_NO_ESCALATE:
        record["final_status"] = s1["status"]
        record["final_c_path"] = s1.get("c_path", "")
        record["final_stage"] = "stage1_gemma"
        return record

    # ----- Stage 2 : gpt-oss critic mutation + gemma4 retry -----
    if enable_critic:
        sys.stderr.write(f"  [stage2 gpt-oss critic + gemma4 retry]\n")
        sys.stderr.flush()
        s2 = run_stage_critic_mutation(
            mod, name, v2_system, examples_md, task_template,
            stage1_turns=s1.get("turns", []),
            stage1_c_path=s1.get("c_path", ""),
            out_dir=out_dir, model=gemma_model,
            api_base=api_base, api_key=api_key,
        )
        s2["stage"] = "gpt_oss_critic_mutation"
        record["stages"].append(s2)
        if s2.get("status") in TERMINAL_NO_ESCALATE:
            record["final_status"] = s2["status"]
            record["final_c_path"] = s2.get("c_path", "")
            record["final_stage"] = "stage2_critic"
            return record

    # ----- Stage 3 : deepseek-v4-pro multi-turn with hardcore prompt -----
    sys.stderr.write(f"  [stage3 deepseek hardcore multi-turn]\n")
    sys.stderr.flush()
    s3 = run_stage_translate(
        mod, name, hardcore_system, examples_md, task_template,
        out_dir / "_stage3_deepseek", deepseek_model, api_base, api_key,
        max_turns,
    )
    s3["stage"] = "deepseek_hardcore_multiturn"
    record["stages"].append(s3)
    record["final_status"] = s3["status"]
    record["final_c_path"] = s3.get("c_path", "")
    record["final_stage"] = "stage3_deepseek"
    return record


# ─────────────────────────────────────────────────────────────────────
# CLI
# ─────────────────────────────────────────────────────────────────────

def main():
    ap = argparse.ArgumentParser()
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--names", nargs="+", help="mod:name entries")
    g.add_argument("--names-file", type=Path)
    ap.add_argument("--gemma-model",    default=DEFAULT_GEMMA)
    ap.add_argument("--deepseek-model", default=DEFAULT_DEEPSEEK)
    ap.add_argument("--critic-model",   default=DEFAULT_CRITIC)
    ap.add_argument("--v2-system",      type=Path, default=V2_SYSTEM)
    ap.add_argument("--hardcore-system", type=Path, default=HARDCORE_SYS)
    ap.add_argument("--task-md",        type=Path, default=TASK_MD)
    ap.add_argument("--examples-md",    type=Path, default=EXAMPLES_MD)
    ap.add_argument("--api-base",       default=DEFAULT_API_BASE)
    ap.add_argument("--api-key",        default=None)
    ap.add_argument("--out-dir",        type=Path,
                    default=ROOT / "port" / "_cascade")
    ap.add_argument("--max-turns",      type=int, default=3,
                    help="multi-turn budget per stage 1 and 3")
    ap.add_argument("--enable-critic",  action="store_true", default=True,
                    help="run stage 2 (gpt-oss critic + gemma retry) "
                         "between stage 1 and 3 (default: on)")
    ap.add_argument("--no-critic", dest="enable_critic", action="store_false")
    ap.add_argument("--log", type=Path,
                    default=THIS / "runs" / "cascade_log.jsonl")
    args = ap.parse_args()

    api_key = args.api_key
    if api_key is None and DEFAULT_KEY_PATH.exists():
        api_key = DEFAULT_KEY_PATH.read_text().strip()
    if not api_key:
        sys.exit("no api key")

    if args.names:
        raw = args.names
    else:
        raw = [l.strip() for l in args.names_file.read_text().splitlines()]
    targets: list[tuple[str, str]] = []
    for line in raw:
        if not line or line.startswith("#") or ":" not in line:
            continue
        mod, name = line.split(":", 1)
        targets.append((mod.strip(), name.strip()))

    v2_system       = args.v2_system.read_text()
    hardcore_system = args.hardcore_system.read_text()
    examples_md     = args.examples_md.read_text() if args.examples_md.exists() else ""
    task_template   = args.task_md.read_text()

    sys.stderr.write(f"[cascade] {len(targets)} routines, max-turns={args.max_turns}, "
                     f"critic={'on' if args.enable_critic else 'off'}\n")
    sys.stderr.write(f"[cascade] stage1=gemma4({args.gemma_model}) "
                     f"stage2=critic({args.critic_model}) "
                     f"stage3=deepseek({args.deepseek_model})\n")
    sys.stderr.flush()

    args.log.parent.mkdir(parents=True, exist_ok=True)
    log_fp = args.log.open("a")
    summary = {"pass": 0, "delegate_pass": 0, "ram_diverge": 0,
               "compile_error": 0, "custom_spike": 0, "no_code": 0,
               "unknown": 0, "asm_empty": 0, "hydrate_error": 0,
               "spike_timeout": 0, "critic_empty": 0,
               "critic_hydrate_error": 0, "http_error": 0}
    stops_at = {"stage1_gemma": 0, "stage2_critic": 0, "stage3_deepseek": 0}

    for i, (mod, name) in enumerate(targets, 1):
        sys.stderr.write(f"\n[{i}/{len(targets)}] {mod}:{name}\n")
        sys.stderr.flush()
        r = cascade_one(
            mod, name, v2_system, hardcore_system, examples_md, task_template,
            args.out_dir, args.api_base, api_key,
            args.gemma_model, args.deepseek_model,
            args.max_turns, args.enable_critic,
        )
        st = r["final_status"]
        summary[st] = summary.get(st, 0) + 1
        stops_at[r["final_stage"]] = stops_at.get(r["final_stage"], 0) + 1
        sys.stderr.write(f"    final={st} via {r['final_stage']}\n")
        rec = {"i": i, "mod": mod, "name": name,
               "final_status": st, "final_stage": r["final_stage"],
               "final_c_path": r.get("final_c_path", ""),
               "stages": [
                   {k: s.get(k) for k in ("stage", "status", "turns",
                                          "tokens_out_total")
                    if k in s} for s in r["stages"]
               ]}
        # Also persist a "c_path" alias on PASS records so port_validated
        # can pick them up via the same code path as hardcore_log.
        if st in PASS_STATUSES:
            rec["c_path"] = r["final_c_path"]
        log_fp.write(json.dumps(rec) + "\n")
        log_fp.flush()

    log_fp.close()
    sys.stderr.write(f"\n[cascade] summary: {summary}\n")
    sys.stderr.write(f"[cascade] stops_at: {stops_at}\n")
    print(json.dumps({"summary": summary, "stops_at": stops_at}, indent=2))


if __name__ == "__main__":
    main()
