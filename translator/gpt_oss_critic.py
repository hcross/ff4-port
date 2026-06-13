#!/usr/bin/env python3
"""gpt-oss-120b critic for prompt mutation (P3 / ADR-004).

Given the current reverser system prompt, a failed translation and its
asm source, ask gpt-oss-120b to propose a NEW full replacement for the
system prompt that would have made the translation succeed without
weakening the prompt for other routines.

Usage:
    python translator/gpt_oss_critic.py \\
        --system-md prompts/history/v0/reverser_system.md \\
        --asm-file upstream/sound/sound.asm \\
        --routine-name PlaySong \\
        --generated-c port/_failed/PlaySong.c \\
        --error-class COMPILE_ERROR \\
        --error-message "PlaySong.c:42: error: ..." \\
        --out-system-md /tmp/proposed_system_v1.md

The critic's reasoning trace is printed to stderr; only the new
system_prompt text is written to --out-system-md.
"""
from __future__ import annotations

import argparse
import json
import os
import sys
import urllib.request
from pathlib import Path


CRITIC_MODEL = "gpt-oss:120b"
DEFAULT_API_BASE = "https://ollama.com/v1"
DEFAULT_API_KEY_PATH = Path.home() / ".ollama" / "ff4-port.api.key"

CRITIC_SYSTEM_PROMPT = """You are a senior prompt engineer auditing a code-generation
pipeline. The pipeline takes a 65816 SNES assembly routine and asks an LLM
(gemma4:31b) to translate it to C. The C must compile against a LakeSnes-based
Snes* struct, follow a strict signature `void <Name>_c(Snes *snes)`, and pass
a runtime fuzz parity test against the original asm.

The system prompt currently produces ~23% PASS. You are given:
  - The CURRENT system prompt (the one being used now, ~340 lines, with
    10 numbered Pitfalls and other key sections).
  - The asm source of a routine that FAILED.
  - The C output gemma4 produced (which failed).
  - The failure class (WRONG_SIGNATURE / COMPILE_ERROR / NO_CODE /
    TRUNCATED / HALLUCINATED / RAM_DIVERGE / ORACLE_BLIND / FAIL).
  - The verbatim error message or evidence.

CRITICAL OUTPUT RULES (your output MUST comply; non-compliance breaks
the loop):

1. Output the COMPLETE new system prompt, verbatim ready to be saved
   as reverser_system.md. Do NOT abbreviate, summarize, or shorten any
   section. The output is loaded as-is into gemma4's context.

2. PRESERVE EVERY SECTION of the current prompt verbatim unless a
   section directly causes this failure. In particular keep ALL the
   numbered Pitfalls (Pitfall 1..10), the API reference, the Output
   format, the Architecture context — copy them character-for-character.

3. Your change should be ADDITIVE: introduce a new numbered Pitfall
   (continue the numbering: 11, 12, ...) OR refine the wording of ONE
   existing pitfall by appending a clarifying example. Do NOT remove
   any pitfall.

4. Your output length must be AT LEAST as long as the input prompt (in
   line count). If you find yourself shortening anything, you are
   doing it wrong — re-emit the omitted content verbatim.

5. Output markdown only, no preamble, no commentary outside the prompt
   itself. Start with the same `# Language requirement` section.
"""


def call_gpt_oss(messages: list[dict], api_base: str, api_key: str,
                  max_output_tokens: int) -> str:
    """POST to OpenAI-compatible /chat/completions and return the assistant
    message content. Returns empty string on extraction failure."""
    payload = {
        "model": CRITIC_MODEL,
        "messages": messages,
        "max_tokens": max_output_tokens,
        "temperature": 0.2,
    }
    req = urllib.request.Request(
        f"{api_base.rstrip('/')}/chat/completions",
        data=json.dumps(payload).encode(),
        headers={
            "Content-Type": "application/json",
            "Authorization": f"Bearer {api_key}",
        },
    )
    try:
        with urllib.request.urlopen(req, timeout=300) as resp:
            data = json.loads(resp.read().decode())
    except Exception as e:
        sys.stderr.write(f"[critic] call failed: {e}\n")
        return ""

    try:
        msg = data["choices"][0]["message"]
        content = msg.get("content") or ""
        if not content:
            # gpt-oss-120b is a reasoning model; fallback to msg.reasoning
            # if max_tokens cut content off (see calibration v2 fix).
            content = msg.get("reasoning") or ""
        finish = data["choices"][0].get("finish_reason", "")
        usage = data.get("usage", {})
        sys.stderr.write(
            f"[critic] finish={finish} prompt_tok={usage.get('prompt_tokens',0)} "
            f"completion_tok={usage.get('completion_tokens',0)}\n"
        )
        return content
    except (KeyError, IndexError):
        sys.stderr.write(f"[critic] unexpected response shape\n")
        return ""


def propose_mutation(
    current_system_md: str,
    asm_excerpt: str,
    routine_name: str,
    generated_c: str,
    error_class: str,
    error_message: str,
    api_base: str,
    api_key: str,
    max_output_tokens: int = 16384,
) -> str:
    """Call the critic. Return the new system prompt (or empty string on
    failure)."""
    user_msg = f"""# Failed routine

Name: `{routine_name}`
Failure class: `{error_class}`

# Asm source

```asm
{asm_excerpt}
```

# C produced by gemma4:31b (FAILED)

```c
{generated_c}
```

# Failure evidence

```
{error_message}
```

# Current system prompt (in use NOW — produces ~23% PASS)

```markdown
{current_system_md}
```

# Your output

Produce the NEW full system prompt that would have helped this routine
succeed. Output markdown only, no preamble.
"""
    messages = [
        {"role": "system", "content": CRITIC_SYSTEM_PROMPT},
        {"role": "user", "content": user_msg},
    ]
    return call_gpt_oss(messages, api_base, api_key, max_output_tokens)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--system-md", required=True, type=Path,
                    help="path to current reverser_system.md")
    ap.add_argument("--asm-file", required=True, type=Path,
                    help="path to upstream/<mod>/<name>.asm (or .asm containing the routine)")
    ap.add_argument("--routine-name", required=True)
    ap.add_argument("--generated-c", required=True, type=Path)
    ap.add_argument("--error-class", required=True)
    ap.add_argument("--error-message", default="")
    ap.add_argument("--out-system-md", required=True, type=Path)
    ap.add_argument("--api-base", default=DEFAULT_API_BASE)
    ap.add_argument("--api-key", default=None)
    ap.add_argument("--max-output-tokens", type=int, default=16384)
    ap.add_argument("--asm-window", type=int, default=200,
                    help="how many lines of asm context around the routine to send")
    args = ap.parse_args()

    api_key = args.api_key or os.environ.get("OPENAI_API_KEY")
    if not api_key and DEFAULT_API_KEY_PATH.exists():
        api_key = DEFAULT_API_KEY_PATH.read_text().strip()
    if not api_key:
        sys.exit("no api key (env OPENAI_API_KEY or ~/.ollama/ff4-port.api.key)")

    current_system_md = args.system_md.read_text()
    asm_full = args.asm_file.read_text()

    # extract a window of asm around the routine label
    lines = asm_full.splitlines()
    start = 0
    for i, line in enumerate(lines):
        if line.startswith(f"{args.routine_name}:"):
            start = i
            break
    asm_excerpt = "\n".join(lines[start : start + args.asm_window])

    generated_c = args.generated_c.read_text() if args.generated_c.exists() else "(empty)"

    new_prompt = propose_mutation(
        current_system_md, asm_excerpt, args.routine_name,
        generated_c, args.error_class, args.error_message,
        args.api_base, api_key, args.max_output_tokens,
    )

    if not new_prompt.strip():
        sys.exit("[critic] empty response — aborting")

    args.out_system_md.parent.mkdir(parents=True, exist_ok=True)
    args.out_system_md.write_text(new_prompt)
    sys.stderr.write(f"[critic] wrote {args.out_system_md} ({len(new_prompt)} bytes)\n")


if __name__ == "__main__":
    main()
