# Multi-model bench results — Ollama Cloud (Pro subscription)

Target: `battle::AICond_02` ($03:BDA9) — 7 asm instructions, fixed output
at `$DE`. Same prompt, same task across all candidates. The auto-spike
generator validates via a 100-trial fuzz (asm vs C, byte-equal on `$DE`).

Baseline: `claude-sonnet-4-5` via Claude Code subscription CLI.

## Results

| Rank | Model | Latency | Output tokens | Spike result | $/translation |
|------|-------|--------:|--------------:|--------------|--------------:|
| 🥇 | **qwen3-coder:480b** | **4.9 s** | **346** | **PASS 100/100** | $0 (Ollama Pro) |
| 🥈 | gemma4:31b | 5.1 s | 572 | PASS 100/100 | $0 (Ollama Pro) |
| 🥉 | devstral-small-2:24b | 9.1 s | 368 | PASS 100/100 | $0 (Ollama Pro) |
| 4 | deepseek-v4-pro | 13.4 s | 1 918 | PASS 100/100 | $0 (Ollama Pro) |
| 5 | minimax-m3 | 49.4 s | 2 356 | PASS 100/100 | $0 (Ollama Pro) |
| 6 | nemotron-3-ultra | 186.0 s | 3 232 | PASS 100/100 | $0 (Ollama Pro) |
| ref | claude-sonnet-4-5 | ~30 s | 5 486 | PASS 1000/1000 | $0.17 (would-be API) |
| — | deepseek-v4-flash | 30.1 s | 3 555 | CUSTOM_SPIKE: yes (self-skipped) | $0 |
| — | gpt-oss:120b | 21.3 s | 1 570 | build error (malformed C) | $0 |
| ✗ | nemotron-3-super | 54.9 s | 4 000 (capped) | no code block extracted | $0 |
| ✗ | glm-5.1 | 43.6 s | 4 000 (capped) | no code block extracted | $0 |
| ✗ | qwen3.5:397b | 63.4 s | 4 000 (capped) | no code block extracted | $0 |
| ✗ | kimi-k2.6 | 13.7 s | 4 000 (capped) | no code block extracted | $0 |
| ✗ | minimax-m3 (round 1) | 84.7 s | 4 000 (capped) | no code block extracted | $0 |
| ✗ | glm-4.7 | 120 s timeout | — | — | $0 |
| ✗ | deepseek-v3.2 | 120 s timeout | — | — | $0 |

The "no code block extracted" failures all hit the `--max-output-tokens 4000`
cap with the body still mid-stream. The two timeouts happened with
`urlopen(timeout=120)` and would likely succeed with the now-bumped 300 s
budget. Worth a re-run when time permits.

## Insights

- **The verbose models are the bad ones.** Claude Sonnet 4.5 produces
  5 486 tokens for a 7-instruction routine. qwen3-coder:480b nails the same
  task in 346 tokens — 16× less. Both pass parity. The Sonnet verbosity is
  ASCII banners + line-by-line transcription that survive the "concise
  mode" instruction.

- **All non-trivial parses produced the same `$BD:A9` bank/offset
  confusion**, regardless of model. The Phase 4.5 Fix 1 (ca65-bridge as
  ground truth) covers all of them transparently.

- **Cost** : the Ollama Pro subscription puts every credible model at $0
  per call. Claude Code subscription gates everything behind a session
  quota (api_error_status=429) we hit after ~3 verbose translations.
  Switching to qwen3-coder:480b removes that constraint entirely.

## Recommendation

Default LLM for new batches:
```bash
OPENAI_API_KEY="$(cat ~/.ollama/<your-key-file>)" \
    python translator/batch_translate.py \
        --module battle --max-functions 20 \
        --llm openai-compat \
        --api-base https://ollama.com/v1 \
        --model qwen3-coder:480b
```

claude-cli stays available as a fallback when you want a longer-context
deliberation or are debugging the prompt.
