# FF4 batch translator

asm 65816 → C pipeline via a **pluggable LLM provider**, with budget
control and a strict non-interactive mode.

## Supported LLM providers

| Provider          | Flag            | Cost          | Cache  | Notes                                  |
|-------------------|-----------------|---------------|--------|----------------------------------------|
| **Claude Code CLI** (default) | `--llm claude-cli`    | Pro/Max subscription  | (auto) | No API key, `claude --print` non-interactive mode |
| Anthropic SDK     | `--llm anthropic-sdk` | pay-per-token         | 90% off | `ANTHROPIC_API_KEY` required, native prompt caching |
| OpenAI-compatible | `--llm openai-compat` | depends on server     | no    | Local Ollama (free), OpenRouter, LM Studio, vLLM |

## Cost model

| Model              | Input  | Output | Cache read | Estimated 30 fns translate |
|--------------------|-------:|-------:|-----------:|---------------------------:|
| claude-haiku-4-5   | $1/M   | $5/M   | $0.10/M    | ~$0.30                     |
| **claude-sonnet-4-6** (default) | $3/M | $15/M | $0.30/M | **~$2**         |
| claude-opus-4-7    | $15/M  | $75/M  | $1.50/M    | ~$15                       |

With prompt caching (system + few-shots ~5k cached tokens):
- 1st call: full rate
- Subsequent calls: -90% on cached tokens
- **Whole battle/ (~30 translate): ~$1-3 estimated**
- **Whole project (~150 translate across 6 modules): ~$5-15 estimated**

## Hard budget cap

The script stops automatically when cumulative cost exceeds `--budget-usd`.
Default: $1.

```bash
# Dry-run with claude CLI (free, default)
python batch_translate.py --module battle --max-functions 5 --dry-run

# Real run with claude CLI (consumes subscription, not API)
python batch_translate.py --module battle --max-functions 3

# Run with Anthropic SDK + hard budget cap
ANTHROPIC_API_KEY=sk-ant-... python batch_translate.py \
    --llm anthropic-sdk \
    --module battle --max-functions 3 \
    --budget-usd 0.50

# Run with local Ollama (free, open model)
python batch_translate.py \
    --llm openai-compat \
    --api-base http://localhost:11434/v1 \
    --model llama3:8b \
    --module battle --max-functions 3

# Run with OpenRouter
python batch_translate.py \
    --llm openai-compat \
    --api-base https://openrouter.ai/api/v1 \
    --api-key $OPENROUTER_KEY \
    --model anthropic/claude-sonnet-4 \
    --module battle --max-functions 3 \
    --budget-usd 1.0
```

## Non-interactive mode

- **No user questions**: all parameters via CLI args
- **Structured output**: JSON per function on stdout (one object per line)
- **Persistent log**: `translator/batch_log.jsonl` (append-only)
- **Exit code**: 0 if OK, non-zero on fatal error (budget, missing API key, etc.)
- **Final summary**: on stderr, does not pollute the JSON stdout

Example stdout (one object per line):
```json
{"name": "CalcHits", "module": "battle", "decision": "translate", "tokens_in": 287, "tokens_cache_read": 5123, "tokens_out": 124, "cost_usd": 0.003, "status": "translated"}
{"name": "CalcDmg", "module": "battle", "decision": "delegate", "cost_usd": 0.0, "status": "delegate_emitted"}
```

## Output

- C code written to `port/<module>/<func>.c`
- Delegated wrappers: one per file, trivial content
- Translations: must be validated next via the parity harness (see Phase 4.3 — auto-spike)

## Safeguards

- `--max-functions N` (default 5)
- `--budget-usd X` (default $1, 0 = no cap)
- `--only-translate` / `--only-delegate` to target a subset
- Dry-run = no API call, estimate only

## Recommended workflow

1. **Always** start with `--dry-run` to see the token estimate
2. Run a real sanity test with `--max-functions 3 --budget-usd 0.10`
3. Inspect the resulting `port/<module>/`
4. If OK, scale up incrementally
