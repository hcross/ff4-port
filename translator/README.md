# FF4 batch translator

Pipeline asm 65816 → C via **LLM pluggable**, avec budget contrôlé et mode
non-interactif.

## Providers LLM supportés

| Provider          | Flag            | Coût          | Cache  | Notes                                  |
|-------------------|-----------------|---------------|--------|----------------------------------------|
| **Claude Code CLI** (default) | `--llm claude-cli`    | abonnement Pro/Max  | (auto) | Pas de gestion clé API, mode `claude --print` non-interactif |
| Anthropic SDK     | `--llm anthropic-sdk` | pay-per-token       | 90% off | `ANTHROPIC_API_KEY` requis, prompt caching natif |
| OpenAI-compatible | `--llm openai-compat` | selon serveur       | non    | Ollama local (gratuit), OpenRouter, LM Studio, vLLM |

## Modèle économique

| Modèle             | Input  | Output | Cache read | Estimation 30 fns translate |
|--------------------|-------:|-------:|-----------:|-----------------------------:|
| claude-haiku-4-5   | $1/M   | $5/M   | $0.10/M    | ~$0.30                       |
| **claude-sonnet-4-6** (défaut) | $3/M | $15/M | $0.30/M | **~$2**             |
| claude-opus-4-7    | $15/M  | $75/M  | $1.50/M    | ~$15                         |

Avec prompt caching (system + few-shots ~5k tokens cachés) :
- 1er call : tarif plein
- Calls suivants : -90 % sur tokens cachés
- **Pour tout battle/ (~30 translate) : ~$1-3 estimé**
- **Pour tout le projet (~150 translate sur 6 modules) : ~$5-15 estimé**

## Hard cap budget

Le script s'arrête automatiquement si le coût cumulé dépasse `--budget-usd`.
Valeur par défaut : $1.

```bash
# Test en mode dry-run avec claude CLI (gratuit, défaut)
python batch_translate.py --module battle --max-functions 5 --dry-run

# Run réel avec claude CLI (consomme abonnement, pas l'API)
python batch_translate.py --module battle --max-functions 3

# Run avec Anthropic SDK + hard cap budget
ANTHROPIC_API_KEY=sk-ant-... python batch_translate.py \
    --llm anthropic-sdk \
    --module battle --max-functions 3 \
    --budget-usd 0.50

# Run avec Ollama local (gratuit, modèle ouvert)
python batch_translate.py \
    --llm openai-compat \
    --api-base http://localhost:11434/v1 \
    --model llama3:8b \
    --module battle --max-functions 3

# Run avec OpenRouter
python batch_translate.py \
    --llm openai-compat \
    --api-base https://openrouter.ai/api/v1 \
    --api-key $OPENROUTER_KEY \
    --model anthropic/claude-sonnet-4 \
    --module battle --max-functions 3 \
    --budget-usd 1.0
```

## Mode non-interactif

- **Aucune question utilisateur** : tous paramètres via CLI args
- **Sortie structurée** : JSON par fonction sur stdout (un objet par ligne)
- **Log persistant** : `translator/batch_log.jsonl` (append-only)
- **Exit code** : 0 si tout OK, non-zéro si erreur fatale (budget, API key manquante, etc.)
- **Récap final** : sur stderr, ne pollue pas le stdout JSON

Exemple de sortie stdout (1 objet par ligne) :
```json
{"name": "CalcHits", "module": "battle", "decision": "translate", "tokens_in": 287, "tokens_cache_read": 5123, "tokens_out": 124, "cost_usd": 0.003, "status": "translated"}
{"name": "CalcDmg", "module": "battle", "decision": "delegate", "cost_usd": 0.0, "status": "delegate_emitted"}
```

## Output

- C code écrit dans `port/<module>/<func>.c`
- Wrappers délégués : un par fichier, contenu trivial
- Translations : à valider ensuite via le harness parity (voir Phase 4.3 — auto-spike)

## Garde-fous

- `--max-functions N` (default 5)
- `--budget-usd X` (default $1)
- `--only-translate` / `--only-delegate` pour cibler un sous-ensemble
- Dry-run = aucune API call, estimation seule

## Workflow recommandé

1. **Toujours** commencer par `--dry-run` pour voir l'estimation tokens
2. Faire un test réel avec `--max-functions 3 --budget-usd 0.10` (sanity)
3. Inspecter le `port/<module>/` produit
4. Si OK, agrandir progressivement le scope
